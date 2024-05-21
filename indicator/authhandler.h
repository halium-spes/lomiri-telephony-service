/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * Authors:
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include <QDBusInterface>
#include <QVariantMap>
#include "indicator/NotificationsInterface.h"
#include "accountentry.h"
#include "notificationmenu.h"

class AuthHandler : public QObject
{
    Q_OBJECT
public:
    explicit AuthHandler(QObject *parent = 0);

public Q_SLOTS:
    void onConnectionStatusChanged(Tp::ConnectionStatus status);
    void actionInvoked(uint id, const QString &actionKey);
    void notificationClosed(uint id, uint reason);
    void clear();
private Q_SLOTS:
    void setupAccounts();

private:
    void processStatusChange(AccountEntry *account, Tp::ConnectionStatus status);
    NotificationMenu mMenuNotification;
    org::freedesktop::Notifications mNotifications;
    QMap<uint, AccountEntry*> mAuthFailureRequests;
    QStringList mIgnoredAccounts;
};

#endif // AUTHHANDLER_H
