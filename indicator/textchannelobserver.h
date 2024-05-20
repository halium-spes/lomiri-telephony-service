/*
 * Copyright (C) 2012-2016 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
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

#ifndef TEXTCHANNELOBSERVER_H
#define TEXTCHANNELOBSERVER_H

#include <libnotify/notify.h>
#include <QContact>
#include <QObject>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/ReceivedMessage>
#include "messagingmenu.h"
#include <History/Thread>

QTCONTACTS_USE_NAMESPACE

class TextChannelObserver : public QObject
{
    Q_OBJECT
public:
    explicit TextChannelObserver(QObject *parent = 0);
    ~TextChannelObserver();

public Q_SLOTS:
    void onTextChannelAvailable(Tp::TextChannelPtr textChannel);
    void sendMessage(NotificationData notificationData);
    void clearNotifications();

protected:
    void showNotificationForFlashMessage(const Tp::ReceivedMessage &message, const QString &accountId);
    void triggerNotificationForMessage(const Tp::TextChannelPtr channel, const Tp::ReceivedMessage &message, const QString &accountId, const QStringList &participantIds = QStringList());
    void showNotificationForCellBroadcast(const Tp::TextChannelPtr channel, const Tp::ReceivedMessage &message, const QString &accountId);
    void showNotificationForMessage(const Tp::TextChannelPtr channel, const Tp::ReceivedMessage &message, const QString &accountId, const QStringList &participantIds = QStringList(), const QContact &contact = QContact());
    void showNotificationForNewGroup(const History::Thread &thread);

protected Q_SLOTS:
    void onTextChannelInvalidated();
    void onMessageReceived(const Tp::ReceivedMessage &message);
    void onPendingMessageRemoved(const Tp::ReceivedMessage &message);
    void onReplyReceived(NotificationData notificationData);
    void onMessageRead(NotificationData notificationData);
    void onMessageSent(Tp::Message, Tp::MessageSendingFlags, QString);
    void onThreadsAdded(History::Threads threads);
    void updateNotifications(const QtContacts::QContact &contact);

private:
    void processMessageReceived(const Tp::ReceivedMessage &message, const Tp::TextChannelPtr &textChannel);
    QList<Tp::TextChannelPtr> mChannels;
    QList<Tp::TextChannelPtr> mFlashChannels;
    QMap<NotifyNotification*, NotificationData*> mNotifications;
    QList<QByteArray> mUnreadMessages;
    QDBusInterface mPowerdIface;
};

#endif // TEXTCHANNELOBSERVER_H
