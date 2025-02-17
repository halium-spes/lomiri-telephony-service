/**
 * Copyright (C) 2013-2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 3, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Tiago Salem Herrmann <tiago.herrmann@canonical.com>
 *          Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 */

// telepathy-ofono
#include "textchannel.h"
#include "include/cellbroadcast-types.h"

QDBusArgument &operator<<(QDBusArgument &argument, const AttachmentStruct &attachment)
{
    argument.beginStructure();
    argument << attachment.id << attachment.contentType << attachment.filePath << attachment.offset << attachment.length;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, AttachmentStruct &attachment)
{
    argument.beginStructure();
    argument >> attachment.id >> attachment.contentType >> attachment.filePath >> attachment.offset >> attachment.length;
    argument.endStructure();
    return argument;
}

MockTextChannel::MockTextChannel(MockConnection *conn, QStringList recipients, uint targetHandle, QObject *parent):
    QObject(parent),
    mConnection(conn),
    mRecipients(recipients),
    mTargetHandle(targetHandle),
    mMessageCounter(1)
{
    qDBusRegisterMetaType<AttachmentStruct>();
    qDBusRegisterMetaType<AttachmentList>();

    Tp::HandleType type = recipients.count() > 1 ? Tp::HandleTypeNone : Tp::HandleTypeContact;


    Tp::BaseChannelPtr baseChannel = Tp::BaseChannel::create(mConnection,
                                                             TP_QT_IFACE_CHANNEL_TYPE_TEXT,
                                                             type,
                                                             targetHandle);
    mBaseChannel = baseChannel;
    Tp::BaseChannelTextTypePtr textType = Tp::BaseChannelTextType::create(baseChannel.data());
    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(textType));

    QStringList supportedContentTypes = QStringList() << "text/plain";

    Tp::UIntList messageTypes = Tp::UIntList() << 
                                Tp::ChannelTextMessageTypeNormal << 
                                Tp::ChannelTextMessageTypeDeliveryReport;
    uint messagePartSupportFlags = 0;
    uint deliveryReportingSupport = Tp::DeliveryReportingSupportFlagReceiveSuccesses;
    mMessagesIface = Tp::BaseChannelMessagesInterface::create(textType.data(),
                                                          supportedContentTypes,
                                                          messageTypes,
                                                          messagePartSupportFlags,
                                                          deliveryReportingSupport);

    mMessagesIface->setSendMessageCallback(Tp::memFun(this,&MockTextChannel::sendMessage));
    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(mMessagesIface));

    mChatStateIface = Tp::BaseChannelChatStateInterface::create();
    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(mChatStateIface));

#if TP_QT_VERSION >= TP_QT_VERSION_CHECK(0, 9, 7)
    // group stuff
    mGroupIface = Tp::BaseChannelGroupInterface::create();
    mGroupIface->setGroupFlags(Tp::ChannelGroupFlagCanAdd);
    mGroupIface->setSelfHandle(conn->selfHandle());
#else
    mGroupIface = Tp::BaseChannelGroupInterface::create(Tp::ChannelGroupFlagCanAdd, conn->selfHandle());
#endif
    mGroupIface->setAddMembersCallback(Tp::memFun(this,&MockTextChannel::onAddMembers));
    mGroupIface->setRemoveMembersCallback(Tp::memFun(this,&MockTextChannel::onRemoveMembers));
    baseChannel->plugInterface(Tp::AbstractChannelInterfacePtr::dynamicCast(mGroupIface));
    addMembers(recipients);

    mTextChannel = Tp::BaseChannelTextTypePtr::dynamicCast(mBaseChannel->interface(TP_QT_IFACE_CHANNEL_TYPE_TEXT));
    mTextChannel->setMessageAcknowledgedCallback(Tp::memFun(this,&MockTextChannel::messageAcknowledged));
    QObject::connect(mBaseChannel.data(), SIGNAL(closed()), this, SLOT(deleteLater()));
}

MockTextChannel::~MockTextChannel()
{
}

Tp::BaseChannelPtr MockTextChannel::baseChannel()
{
    return mBaseChannel;
}

void MockTextChannel::messageAcknowledged(const QString &id)
{
    Q_EMIT messageRead(id);
}

QString MockTextChannel::sendMessage(const Tp::MessagePartList& message, uint flags, Tp::DBusError* error)
{
    Tp::MessagePart header = message.at(0);

    static int serial = 0;

    // FIXME: check what other data we need to emit in the signal
    QString id = QString("sentmessage%1").arg(serial++);
    QString messageText;
    QVariantMap properties;
    QMap<QString, QDBusVariant>::const_iterator it = header.constBegin();
    while (it != header.constEnd()) {
        properties[it.key()] = it.value().variant().toString();
        it++;
    }
    properties["SentTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    properties["Recipients"] = mRecipients;
    properties["Id"] = id;

    QVariantList attachments;
    Tp::MessagePartList messageList = message;
    messageList.pop_front();
    Q_FOREACH(const Tp::MessagePart& messagePart, messageList) {
        QVariantMap attachment;
        if (messagePart.contains("content-type") && messagePart["content-type"].variant().toString().startsWith("text/")) {
            messageText = messagePart["content"].variant().toString();
            continue;
        }
        QMap<QString, QDBusVariant>::const_iterator it = messagePart.constBegin();
        while (it != messagePart.constEnd()) {
            attachment[it.key()] = it.value().variant().toString();
            it++;
        }
        attachments << attachment;
    }

    Q_EMIT messageSent(messageText, attachments, properties);

    QTimer *deliveryReportTimer = new QTimer(this);
    deliveryReportTimer->setSingleShot(true);
    deliveryReportTimer->setInterval(100);
    connect(deliveryReportTimer, &QTimer::timeout, [id, deliveryReportTimer, this] {
        this->placeDeliveryReport(id, "sent");
        deliveryReportTimer->deleteLater();
    });
    deliveryReportTimer->start();

    return id;
}

void MockTextChannel::placeDeliveryReport(const QString &messageId, const QString &status)
{
    Tp::DeliveryStatus delivery_status;
    if (status == "sent") {
        delivery_status = Tp::DeliveryStatusDelivered;
    } else if(status == "failed") {
        delivery_status = Tp::DeliveryStatusPermanentlyFailed;
    } else if(status == "pending") {
        delivery_status = Tp::DeliveryStatusAccepted;
    } else {
        delivery_status = Tp::DeliveryStatusUnknown;
    }

    Tp::MessagePartList partList;
    Tp::MessagePart header;
    header["message-sender"] = QDBusVariant(mTargetHandle);
    // FIXME: fix it
    header["message-sender-id"] = QDBusVariant(mRecipients.first());
    header["message-type"] = QDBusVariant(Tp::ChannelTextMessageTypeDeliveryReport);
    header["delivery-status"] = QDBusVariant(delivery_status);
    header["delivery-token"] = QDBusVariant(messageId);
    partList << header;
    mTextChannel->addReceivedMessage(partList);
}

void MockTextChannel::messageReceived(const QString &message, const QVariantMap &info)
{
    Tp::MessagePartList partList;

    Tp::MessagePart body;
    body["content-type"] = QDBusVariant("text/plain");
    body["content"] = QDBusVariant(message);

    Tp::MessagePart header;
    header["message-token"] = QDBusVariant(info["SentTime"].toString() +"-" + QString::number(mMessageCounter++));
    header["message-received"] = QDBusVariant(QDateTime::currentDateTime().toTime_t());
    header["message-sent"] = QDBusVariant(QDateTime::fromString(info["SentTime"].toString(), Qt::ISODate).toTime_t());
    header["message-sender"] = QDBusVariant(mTargetHandle);
    header["message-sender-id"] = QDBusVariant(mRecipients.first());
    header["message-type"] = QDBusVariant(Tp::ChannelTextMessageTypeNormal);
    partList << header << body;

    mTextChannel->addReceivedMessage(partList);
}

void MockTextChannel::cellBroadcastReceived(const QString &message, CellBroadcast::Type type, quint16 topic)
{
    Tp::MessagePartList partList;

    Tp::MessagePart body;
    body["content-type"] = QDBusVariant("text/plain");
    body["content"] = QDBusVariant(message);

    Tp::MessagePart header;
    header["message-token"] = QDBusVariant(QDateTime::currentDateTime().toTime_t());
    header["message-received"] = QDBusVariant(QDateTime::currentDateTime().toTime_t());
    header["message-sent"] = QDBusVariant(QDateTime::currentDateTime().toTime_t());
    header["message-sender"] = QDBusVariant(mTargetHandle);
    header["message-sender-id"] = QDBusVariant(CellBroadcast::CELLBROADCAST_IDENTIFIER);
    header[CellBroadcast::CELLBROADCAST_IDENTIFIER_TYPE] = QDBusVariant(type);
    header["message-type"] = QDBusVariant(Tp::ChannelTextMessageTypeNotice);
    header["subject"] = QDBusVariant(topic);
    partList << header << body;

    mTextChannel->addReceivedMessage(partList);
}

void MockTextChannel::mmsReceived(const QString &id, const QVariantMap &properties)
{
    Tp::MessagePartList message;
    QString subject = properties["Subject"].toString();
    QString smil = properties["Smil"].toString();

    Tp::MessagePart header;
    header["message-token"] = QDBusVariant(id);
    header["message-sender"] = QDBusVariant(mTargetHandle);
    header["message-received"] = QDBusVariant(QDateTime::fromString(properties["Date"].toString(), Qt::ISODate).toTime_t());
    header["message-sent"] = QDBusVariant(QDateTime::fromString(properties["SentTime"].toString(), Qt::ISODate).toTime_t());
    header["message-type"] = QDBusVariant(Tp::DeliveryStatusDelivered);
    if (!subject.isEmpty())
    {
        header["subject"] = QDBusVariant(subject);
    }
    message << header;
    AttachmentList mmsdAttachments = qdbus_cast<AttachmentList>(properties["Attachments"]);
    Q_FOREACH(const AttachmentStruct &attachment, mmsdAttachments) {
        QFile attachmentFile(attachment.filePath);
        if (!attachmentFile.open(QIODevice::ReadOnly)) {
            qWarning() << "fail to load attachment" << attachmentFile.errorString() << attachment.filePath;
            continue;
        }
        // FIXME check if we managed to read the total attachment file
        attachmentFile.seek(attachment.offset);
        QByteArray fileData = attachmentFile.read(attachment.length);
        Tp::MessagePart part;
        part["content-type"] =  QDBusVariant(attachment.contentType);
        part["identifier"] = QDBusVariant(attachment.id);
        part["content"] = QDBusVariant(fileData);
        part["size"] = QDBusVariant(attachment.length);

        message << part;
    }

    if (!smil.isEmpty()) {
        Tp::MessagePart part;
        part["content-type"] =  QDBusVariant(QString("application/smil"));
        part["identifier"] = QDBusVariant(QString("smil"));
        part["content"] = QDBusVariant(smil);
        part["size"] = QDBusVariant(smil.size());
        message << part;
    }

    mTextChannel->addReceivedMessage(message);
}

void MockTextChannel::addMembers(QStringList recipients)
{
    Tp::UIntList handles;
    Q_FOREACH(const QString &recipient, recipients) {
        uint handle = mConnection->ensureHandle(recipient);
        handles << handle;
        if (!mRecipients.contains(recipient)) {
            mRecipients << recipient;
        }
        if (!mMembers.contains(handle)) {
            mMembers << handle;
        }
    }
#if TP_QT_VERSION >= TP_QT_VERSION_CHECK(0, 9, 7)
    mGroupIface->setMembers(Tp::UIntList() << mGroupIface->members() << handles, QVariantMap());
#else
    mGroupIface->addMembers(handles, recipients);
#endif
}

QStringList MockTextChannel::recipients() const
{
    return mRecipients;
}

Tp::UIntList MockTextChannel::members()
{
    return mMembers;
}

void MockTextChannel::onAddMembers(const Tp::UIntList &handles, const QString &message, Tp::DBusError *error)
{
    addMembers(mConnection->inspectHandles(Tp::HandleTypeContact, handles, error));
}

#if TP_QT_VERSION >= TP_QT_VERSION_CHECK(0, 9, 7)
void MockTextChannel::onRemoveMembers(const Tp::UIntList &handles, const QString &message, uint reason, Tp::DBusError *error)
#else
void MockTextChannel::onRemoveMembers(const Tp::UIntList &handles, const QString &message, Tp::DBusError *error)
#endif
{
    Q_FOREACH(uint handle, handles) {
        Q_FOREACH(const QString &recipient, mConnection->inspectHandles(Tp::HandleTypeContact, Tp::UIntList() << handle, error)) {
            mRecipients.removeAll(recipient);
        }
        mMembers.removeAll(handle);
    }
#if TP_QT_VERSION >= TP_QT_VERSION_CHECK(0, 9, 7)
    Tp::UIntList members = mGroupIface->members();
    Q_FOREACH(uint handle, handles) {
        members.removeAll(handle);
    }
    mGroupIface->setMembers(members, QVariantMap());
#else
    mGroupIface->removeMembers(handles);
#endif
}

void MockTextChannel::changeChatState(const QString &userId, int state)
{
    Q_EMIT mChatStateIface->chatStateChanged(mConnection->ensureHandle(userId), state);
}
