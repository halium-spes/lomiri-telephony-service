/*
 * Copyright (C) 2014-2017 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Martti Piirainen <martti.piirainen@canonical.com>
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

#include "tonegenerator.h"

#include <QDebug>
#include <QTimer>
#include <QDBusMessage>
#include <QDBusConnection>

/* Use tone-generator (tonegend) via D-Bus */
#define TONEGEN_DBUS_SERVICE_NAME "com.Nokia.Telephony.Tones"
#define TONEGEN_DBUS_OBJ_PATH "/com/Nokia/Telephony/Tones"
#define TONEGEN_DBUS_IFACE_NAME TONEGEN_DBUS_SERVICE_NAME

ToneGenerator::ToneGenerator(QObject *parent) :
    QObject(parent), mDTMFPlaybackTimer(nullptr), mWaitingPlaybackTimer(new QTimer(this))
{
    // the waiting tone is played in loop
    connect(mWaitingPlaybackTimer, SIGNAL(timeout()), this, SLOT(playWaitingTone()));
    mWaitingPlaybackTimer->setSingleShot(true);
}

ToneGenerator::~ToneGenerator()
{
    this->stopDTMFTone();
    this->stopWaitingTone();
}

ToneGenerator *ToneGenerator::instance()
{
    static ToneGenerator *self = new ToneGenerator();
    return self;
}

bool ToneGenerator::startEventTone(uint key)
{
    QDBusMessage startMsg = QDBusMessage::createMethodCall(
            TONEGEN_DBUS_SERVICE_NAME,
            TONEGEN_DBUS_OBJ_PATH,
            TONEGEN_DBUS_IFACE_NAME,
            "StartEventTone" );
    QList<QVariant> toneArgs;
    toneArgs << QVariant((uint)key);
    toneArgs << QVariant((int)0);  // volume is ignored
    toneArgs << QVariant((uint)0); // duration is ignored
    startMsg.setArguments(toneArgs);
    return QDBusConnection::sessionBus().send(startMsg);
}

void ToneGenerator::playDTMFTone(uint key)
{
    qDebug() << __PRETTY_FUNCTION__ << key;
    if (key > 11) {
        qDebug() << "Invalid DTMF tone, ignore.";
        return;
    }

    if (startEventTone(key)) {
        if (!mDTMFPlaybackTimer) {
            mDTMFPlaybackTimer = new QTimer(this);
            mDTMFPlaybackTimer->setSingleShot(true);
            connect(mDTMFPlaybackTimer, SIGNAL(timeout()), this, SLOT(stopDTMFTone()));
        }
        mDTMFPlaybackTimer->start(DTMF_LOCAL_PLAYBACK_DURATION);
    }
}

void ToneGenerator::stopTone()
{
    QDBusConnection::sessionBus().send(
        QDBusMessage::createMethodCall(
            TONEGEN_DBUS_SERVICE_NAME,
            TONEGEN_DBUS_OBJ_PATH,
            TONEGEN_DBUS_IFACE_NAME,
            "StopTone" ));
}

void ToneGenerator::stopDTMFTone()
{
    stopTone();
    if (mDTMFPlaybackTimer) {
        mDTMFPlaybackTimer->stop();
    }
}

void ToneGenerator::playWaitingTone()
{
    if (mWaitingPlaybackTimer->isActive()) {
        stopTone();
    }

    if (startEventTone(WAITING_TONE)) {
        mWaitingPlaybackTimer->start(WAITING_PLAYBACK_DURATION);
    }
}

void ToneGenerator::stopWaitingTone()
{
    stopTone();
    mWaitingPlaybackTimer->stop();
}

void ToneGenerator::playCallEndedTone()
{
    startEventTone(CALL_ENDED_TONE);
    QTimer::singleShot(2000, this, SLOT(stopTone()));
}

void ToneGenerator::playDialingTone()
{
    startEventTone(DIALING_TONE);
}

void ToneGenerator::playRingingTone()
{
    startEventTone(RINGING_TONE);
}
