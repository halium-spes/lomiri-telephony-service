/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
 *
 * Authors:
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

#ifndef CHATENTRY_H
#define CHATENTRY_H

#include <QObject>
#include <QQmlParserStatus>
#include <QQmlListProperty>
#include <TelepathyQt/TextChannel>
#include "rolesinterface.h"
#include "participant.h"

class AccountEntry;
class Participant;

typedef QMap<uint,uint> RolesMap;
Q_DECLARE_METATYPE(RolesMap)

class ContactChatState : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString contactId READ contactId CONSTANT)
    Q_PROPERTY(int state READ state WRITE setState NOTIFY stateChanged)
public:
    ContactChatState(const QString &contactId, int state) : mContactId(contactId), mState(state) {}
    QString contactId() { return mContactId; }
    int state() { return mState; }
    void setState(int state) {
        mState = state;
        Q_EMIT stateChanged();
    }
Q_SIGNALS:
    void stateChanged();

private:
    QString mContactId;
    int mState;
};

typedef QList<ContactChatState* > ContactChatStates;

class ChatEntry : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(ChatType chatType READ chatType WRITE setChatType NOTIFY chatTypeChanged)
    Q_PROPERTY(QStringList participantIds READ participantIds WRITE setParticipantIds NOTIFY participantIdsChanged)
    Q_PROPERTY(QQmlListProperty<Participant> participants READ participants NOTIFY participantsChanged)
    Q_PROPERTY(QQmlListProperty<Participant> localPendingParticipants READ localPendingParticipants NOTIFY localPendingParticipantsChanged)
    Q_PROPERTY(QQmlListProperty<Participant> remotePendingParticipants READ remotePendingParticipants NOTIFY remotePendingParticipantsChanged)
    Q_PROPERTY(QString roomName READ roomName WRITE setRoomName NOTIFY roomNameChanged)
    Q_PROPERTY(QString chatId READ chatId WRITE setChatId NOTIFY chatIdChanged)
    Q_PROPERTY(QString accountId READ accountId WRITE setAccountId NOTIFY accountIdChanged)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QQmlListProperty<ContactChatState> chatStates READ chatStates NOTIFY chatStatesChanged)
    Q_PROPERTY(bool autoRequest READ autoRequest WRITE setAutoRequest CONSTANT)
    Q_PROPERTY(bool canUpdateConfiguration READ canUpdateConfiguration NOTIFY canUpdateConfigurationChanged)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(uint groupFlags READ groupFlags NOTIFY groupFlagsChanged);
    Q_PROPERTY(uint selfContactRoles READ selfContactRoles NOTIFY selfContactRolesChanged);

    Q_ENUMS(ChatType)
    Q_ENUMS(ChatState)
    Q_ENUMS(ChannelGroupFlag)
public:
    enum ChatType {
        ChatTypeNone    = Tp::HandleTypeNone,
        ChatTypeContact = Tp::HandleTypeContact,
        ChatTypeRoom    = Tp::HandleTypeRoom
    };

    enum ChatState {
        ChannelChatStateGone      = Tp::ChannelChatStateGone,
        ChannelChatStateInactive  = Tp::ChannelChatStateInactive,
        ChannelChatStateActive    = Tp::ChannelChatStateActive,
        ChannelChatStatePaused    = Tp::ChannelChatStatePaused,
        ChannelChatStateComposing = Tp::ChannelChatStateComposing
    };

    enum ChannelGroupFlag
    {
        ChannelGroupFlagCanAdd = Tp::ChannelGroupFlagCanAdd,
        ChannelGroupFlagCanRemove = Tp::ChannelGroupFlagCanRemove,
        ChannelGroupFlagCanRescind = Tp::ChannelGroupFlagCanRescind,
        ChannelGroupFlagMessageAdd = Tp::ChannelGroupFlagMessageAdd,
        ChannelGroupFlagMessageRemove = Tp::ChannelGroupFlagMessageRemove,
        ChannelGroupFlagMessageAccept = Tp::ChannelGroupFlagMessageAccept,
        ChannelGroupFlagMessageReject = Tp::ChannelGroupFlagMessageReject,
        ChannelGroupFlagMessageRescind = Tp::ChannelGroupFlagMessageRescind,
        ChannelGroupFlagChannelSpecificHandles = Tp::ChannelGroupFlagChannelSpecificHandles,
        ChannelGroupFlagOnlyOneGroup = Tp::ChannelGroupFlagOnlyOneGroup,
        ChannelGroupFlagHandleOwnersNotAvailable = Tp::ChannelGroupFlagHandleOwnersNotAvailable,
        ChannelGroupFlagProperties = Tp::ChannelGroupFlagProperties,
        ChannelGroupFlagMembersChangedDetailed = Tp::ChannelGroupFlagMembersChangedDetailed,
        ChannelGroupFlagMessageDepart = Tp::ChannelGroupFlagMessageDepart
    };

    explicit ChatEntry(QObject *parent = 0);
    ~ChatEntry();
    QStringList participantIds() const;
    void setParticipantIds(const QStringList &participantIds);
    QQmlListProperty<Participant> participants();
    QQmlListProperty<Participant> localPendingParticipants();
    QQmlListProperty<Participant> remotePendingParticipants();
    static int participantsCount(QQmlListProperty<Participant> *p);
    static Participant *participantsAt(QQmlListProperty<Participant> *p, int index);
    ChatType chatType() const;
    void setChatType(ChatType type);
    QString chatId() const;
    void setChatId(const QString &id);
    QString accountId() const;
    void setAccountId(const QString &id);
    QString roomName() const;
    void setRoomName(const QString &name);
    QString title() const;
    void setTitle(const QString & title);
    void setAutoRequest(bool autoRequest);
    bool autoRequest() const;
    bool canUpdateConfiguration() const;
    QQmlListProperty<ContactChatState> chatStates();
    static int chatStatesCount(QQmlListProperty<ContactChatState> *p);
    static ContactChatState *chatStatesAt(QQmlListProperty<ContactChatState> *p, int index);
    uint groupFlags() const;
    uint selfContactRoles() const;

    // QML parser status
    bool isActive() const;
    void classBegin();
    void componentComplete();

    QList<Participant*> allParticipants() const;

public Q_SLOTS:
    // FIXME: void or return something?
    void sendMessage(const QString &accountId, const QString &message, const QVariant &attachments = QVariant(), const QVariantMap &properties = QVariantMap());
    void setChatState(ChatState state);

    bool destroyRoom();
    void inviteParticipants(const QStringList &participantIds, const QString &message = QString());
    void removeParticipants(const QStringList &participantIds, const QString &message = QString());

    void startChat();
    bool leaveChat(const QString &message = QString());

protected:
    void setChannels(const QList<Tp::TextChannelPtr> &channels);
    void addChannel(const Tp::TextChannelPtr &channel);

    QVariantMap generateProperties() const;

    void clearParticipants();
    void updateParticipants(QList<Participant*> &list, const Tp::Contacts &added, const Tp::Contacts &removed, AccountEntry *account, Participant::ParticipantState = Participant::ParticipantStateRegular);

private Q_SLOTS:
    void onTextChannelAvailable(const Tp::TextChannelPtr &channel);
    void onChannelInvalidated();
    void onChatStateChanged(const Tp::ContactPtr &contact, Tp::ChannelChatState state);
    void onRoomPropertiesChanged(const QVariantMap &changed,const QStringList &invalidated);
    void onSendingMessageFinished();
    void onGroupMembersChanged(const Tp::Contacts &groupMembersAdded,
                               const Tp::Contacts &groupLocalPendingMembersAdded,
                               const Tp::Contacts &groupRemotePendingMembersAdded,
                               const Tp::Contacts &groupMembersRemoved,
                               const Tp::Channel::GroupMemberChangeDetails &details);
    void onChatStartingFinished();
    void onRolesChanged(const HandleRolesMap &added, const HandleRolesMap &removed);
    void onAccountActiveChanged();

Q_SIGNALS:
    void chatTypeChanged();
    void chatIdChanged();
    void accountIdChanged();
    void chatStatesChanged();
    void participantIdsChanged();
    void participantsChanged();
    void localPendingParticipantsChanged();
    void remotePendingParticipantsChanged();
    void roomNameChanged();
    void titleChanged();
    void canUpdateConfigurationChanged();
    void inviteParticipantsFailed();
    void removeParticipantsFailed();
    void setTitleFailed();
    void activeChanged();
    void groupFlagsChanged();
    void selfContactRolesChanged();

    void messageSent(const QString &accountId, const QString &messageId, const QVariantMap &properties);
    void messageSendingFailed(const QString &accountId, const QString &messageId, const QVariantMap &properties);

    void chatReady();
    void startChatFailed();

    void participantAdded(Participant *participant);
    void participantRemoved(Participant *participant);

private:
    QList<Tp::TextChannelPtr> mChannels;
    QStringList mParticipantIds;
    QList<Participant*> mParticipants;
    QList<Participant*> mLocalPendingParticipants;
    QList<Participant*> mRemotePendingParticipants;
    QMap<QString, ContactChatState*> mChatStates;
    QString mRoomName;
    QString mTitle;
    QString mChatId;
    QString mAccountId;
    ChatType mChatType;
    bool mAutoRequest;
    bool mCanUpdateConfiguration;
    uint mSelfContactRoles;
    Tp::Client::ChannelInterfaceRoomInterface *roomInterface;
    Tp::Client::ChannelInterfaceRoomConfigInterface *roomConfigInterface;
    Tp::Client::ChannelInterfaceSubjectInterface *subjectInterface;
    ChannelInterfaceRolesInterface *rolesInterface;
    RolesMap mRolesMap;
};

#endif // CHATENTRY_H
