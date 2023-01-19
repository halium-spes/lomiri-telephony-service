/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *
 * This file is part of telephony-service.
 *
 * telephony-service is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * telephony-service is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RINGTONE_H
#define RINGTONE_H

#include <QObject>
#include <QThread>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QFile>
#include <QFeedbackHapticsEffect>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QtDBus/QDBusInterface>
#include <QTimer>
#include <unistd.h>
#include <sys/types.h>


class RingtoneWorker : public QObject
{
    Q_OBJECT
public:
    RingtoneWorker(QObject *parent = 0);

public Q_SLOTS:
    void playIncomingCallSound();
    void stopIncomingCallSound();
    void playIncomingMessageSound();
    void stopIncomingMessageSound();
    void playIncomingEmergencySound();
    void playIncomingWarningSound();
    void stopSound();

private:
    void playAlertSound(const QString &soundFile);
    QMediaPlayer *mCallAudioPlayer;
    QMediaPlaylist mCallAudioPlaylist;

    QMediaPlayer *mMessageAudioPlayer;
};

class Ringtone : public QObject
{
    Q_OBJECT
public:
    ~Ringtone();
    static Ringtone *instance();

public Q_SLOTS:
    void playIncomingCallSound();
    void stopIncomingCallSound();

    void playIncomingMessageSound();
    void stopIncomingMessageSound();

    void playIncomingEmergencySound();
    void playIncomingWarningSound();

    void stopSound();

private Q_SLOTS:
    void vibrate();

private:
    explicit Ringtone(QObject *parent = 0);
    void startVibrate(int nbCycle, int duration, int interval);
    QFeedbackHapticsEffect mVibrateEffect;
    int mNbVibrateCycle;
    QTimer mVibrateTimer;
    RingtoneWorker *mWorker;
    QThread mThread;
};

#endif // RINGTONE_H
