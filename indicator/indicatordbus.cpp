/*
 * Copyright (C) 2012-2016 Canonical, Ltd.
 *
 * Authors:
 *  Ugo Riboni <ugo.riboni@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "indicatordbus.h"
#include "indicatoradaptor.h"
#include "messagingmenu.h"

// Qt
#include <QtDBus/QDBusConnection>

static const char* DBUS_SERVICE = "com.lomiri.TelephonyServiceIndicator";
static const char* DBUS_OBJECT_PATH = "/com/lomiri/TelephonyServiceIndicator";

IndicatorDBus::IndicatorDBus(QObject* parent)
: QObject(parent)
{
}

IndicatorDBus::~IndicatorDBus()
{
}

bool
IndicatorDBus::connectToBus()
{
    new TelephonyServiceIndicatorAdaptor(this);
    QDBusConnection::sessionBus().registerObject(DBUS_OBJECT_PATH, this);
    return QDBusConnection::sessionBus().registerService(DBUS_SERVICE);
}

void IndicatorDBus::ClearNotifications()
{
    Q_EMIT clearNotificationsRequested();
}

void IndicatorDBus::ClearCallNotification(const QString &targetId, const QString &accountId)
{
    MessagingMenu::instance()->removeCall(targetId, accountId);
}

