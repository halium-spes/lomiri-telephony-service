/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of lomiri-telephony-service.
 *
 * lomiri-telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * lomiri-telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "greetercontacts.h"
#include "ringtone.h"
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

#define CUSTOM_RINGTONE_SUBDIR "sounds/ringtones"

RingtoneWorker::RingtoneWorker(QObject *parent) :
    QObject(parent), mCallAudioPlayer(NULL), mCallAudioPlaylist(this),
    mMessageAudioPlayer(NULL)
{
    mCallAudioPlaylist.setPlaybackMode(QMediaPlaylist::Loop);
    mCallAudioPlaylist.setCurrentIndex(0);
}

void RingtoneWorker::playIncomingCallSound(const QString &customSound)
{
    if (!qgetenv("PA_DISABLED").isEmpty()) {
        return;
    }

    if (GreeterContacts::instance()->silentMode()) {
        return;
    }

    // force delete all media player instances
    stopIncomingCallSound();

    QString callSound  = GreeterContacts::instance()->incomingCallSound();
    if (!customSound.isEmpty()) {
        QFileInfo file(customSound);
        if (file.exists() && file.isFile()) {

            // mime database to detect file type
            QMimeDatabase db;

            // the mime type (to test if it is an audio file
            QMimeType type = db.mimeTypeForFile(customSound);

            if(type.name().startsWith("audio")){
                callSound = customSound;
            }
        }
    }
    qDebug() << "playIncomingCallSound" << callSound;

    mCallAudioPlaylist.addMedia(QUrl::fromLocalFile(callSound));
    mCallAudioPlayer = new QMediaPlayer(this);
    mCallAudioPlayer->setAudioRole(QAudio::RingtoneRole);
    mCallAudioPlayer->setPlaylist(&mCallAudioPlaylist);
    mCallAudioPlayer->play();
}

void RingtoneWorker::stopIncomingCallSound()
{
    if (mCallAudioPlayer) {
        // WORKAROUND: if we call stop and the stream is already over, qmediaplayer plays again.
        mCallAudioPlayer->pause();
        mCallAudioPlayer->deleteLater();
        mCallAudioPlayer = NULL;
    }
    mCallAudioPlaylist.clear();
}

void RingtoneWorker::playIncomingMessageSound(const QString &customSound)
{
    if (!qgetenv("PA_DISABLED").isEmpty()) {
        return;
    }

    if (GreeterContacts::instance()->silentMode()) {
        return;
    }

    // Re-create if in error state. A typical case is when media-hub-server has
    // crashed and we need to start from a clean slate.
    if (mMessageAudioPlayer && mMessageAudioPlayer->error()) {
        qDebug() << "mMessageAudioPlayer in error state ("
                 << mMessageAudioPlayer->error() << "), recreating";

        mMessageAudioPlayer->deleteLater();
        mMessageAudioPlayer = NULL;
    }

    if (!mMessageAudioPlayer) {
        mMessageAudioPlayer = new QMediaPlayer(this);
        mMessageAudioPlayer->setAudioRole(QAudio::NotificationRole);
    }

    // WORKAROUND: there is a bug in qmediaplayer/(media-hub?) that never goes into Stopped mode.
    if (mMessageAudioPlayer->duration() == mMessageAudioPlayer->position()) {
        mMessageAudioPlayer->stop();
    }

    if (mMessageAudioPlayer->state() == QMediaPlayer::PlayingState) {
        return;
    }

    QString messageSound  = GreeterContacts::instance()->incomingMessageSound();
    if (!customSound.isEmpty()) {
        QFileInfo file(customSound);
        if (file.exists() && file.isFile()) {

            // mime database to detect file type
            QMimeDatabase db;

            // the mime type (to test if it is an audio file
            QMimeType type = db.mimeTypeForFile(customSound);

            if(type.name().startsWith("audio")){
                messageSound = customSound;
            }
        }
    }
    qDebug() << "playIncomingMessageSound" << messageSound;

    mMessageAudioPlayer->setMedia(QUrl::fromLocalFile(messageSound));
    mMessageAudioPlayer->play();
}

void RingtoneWorker::stopIncomingMessageSound()
{
    stopSound();
}

void RingtoneWorker::playIncomingEmergencySound()
{
    playAlertSound(GreeterContacts::instance()->incomingEmergencySound());
}

void RingtoneWorker::playIncomingWarningSound()
{
    playAlertSound(GreeterContacts::instance()->incomingWarningSound());
}

void RingtoneWorker::stopSound()
{
    if (mMessageAudioPlayer) {
        mMessageAudioPlayer->pause();
        mMessageAudioPlayer->deleteLater();
        mMessageAudioPlayer = NULL;
    }
}

void RingtoneWorker::playAlertSound(const QString &soundFile)
{
    if (!qgetenv("PA_DISABLED").isEmpty()) {
        return;
    }

    // Re-create if in error state. A typical case is when media-hub-server has
    // crashed and we need to start from a clean slate.
    if (mMessageAudioPlayer && mMessageAudioPlayer->error()) {
        qDebug() << "mMessageAudioPlayer in error state ("
                 << mMessageAudioPlayer->error() << "), recreating";

        mMessageAudioPlayer->deleteLater();
        mMessageAudioPlayer = NULL;
    }

    if (!mMessageAudioPlayer) {
        mMessageAudioPlayer = new QMediaPlayer(this);
        mMessageAudioPlayer->setAudioRole(QAudio::AlarmRole);
    }

    // WORKAROUND: there is a bug in qmediaplayer/(media-hub?) that never goes into Stopped mode.
    if (mMessageAudioPlayer->duration() == mMessageAudioPlayer->position()) {
        mMessageAudioPlayer->stop();
    }

    if (mMessageAudioPlayer->state() == QMediaPlayer::PlayingState) {
        return;
    }

    mMessageAudioPlayer->setMedia(QUrl::fromLocalFile(soundFile));
    mMessageAudioPlayer->play();
}

Ringtone::Ringtone(QObject *parent) :
    QObject(parent), mNbVibrateCycle(0)
{
    mWorker = new RingtoneWorker();
    mWorker->moveToThread(&mThread);
    mThread.start();
    connect(&mVibrateTimer, SIGNAL(timeout()), this, SLOT(vibrate()));
}

Ringtone::~Ringtone()
{
    mThread.quit();
    mThread.wait();
}

Ringtone *Ringtone::instance()
{
    static Ringtone *self = new Ringtone();
    return self;
}


void Ringtone::startVibrate(int nbCycle, int duration, int interval)
{
    mVibrateTimer.stop();
    mVibrateEffect.stop();

    mNbVibrateCycle = nbCycle;
    mVibrateEffect.setDuration(duration);
    mVibrateEffect.setIntensity(1);
    //immediate vibrate
    vibrate();
    mVibrateTimer.start(interval);
}

void Ringtone::vibrate()
{
    if (mNbVibrateCycle <= 0) {
        mVibrateTimer.stop();
        return;
    }

    mVibrateEffect.stop();
    mVibrateEffect.start();
    mNbVibrateCycle--;
}

void Ringtone::playIncomingCallSound(const QString &customSound)
{
    QMetaObject::invokeMethod(mWorker, "playIncomingCallSound", Qt::QueuedConnection, Q_ARG(QString, customSound));
}

void Ringtone::stopIncomingCallSound()
{
    QMetaObject::invokeMethod(mWorker, "stopIncomingCallSound", Qt::QueuedConnection);
}

void Ringtone::playIncomingMessageSound(const QString &customSound)
{
    if (GreeterContacts::instance()->incomingMessageVibrate() && !mVibrateTimer.isActive()) {
        startVibrate(1, 500, 1);
    }

    QMetaObject::invokeMethod(mWorker, "playIncomingMessageSound", Qt::QueuedConnection, Q_ARG(QString, customSound));
}

void Ringtone::stopIncomingMessageSound()
{
    QMetaObject::invokeMethod(mWorker, "stopIncomingMessageSound", Qt::QueuedConnection);
}

void Ringtone::playIncomingEmergencySound()
{
    startVibrate(10, 3000, 4000);

    QMetaObject::invokeMethod(mWorker, "playIncomingEmergencySound", Qt::QueuedConnection);
}

void Ringtone::playIncomingWarningSound()
{
    if (!mVibrateTimer.isActive()) {
        startVibrate(4, 3000, 4000);
    }

    QMetaObject::invokeMethod(mWorker, "playIncomingWarningSound", Qt::QueuedConnection);
}

void Ringtone::stopSound()
{
    mVibrateEffect.stop();
    mVibrateTimer.stop();
    QMetaObject::invokeMethod(mWorker, "stopSound", Qt::QueuedConnection);
}
