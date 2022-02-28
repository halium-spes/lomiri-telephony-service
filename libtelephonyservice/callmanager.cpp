/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Tiago Salem Herrmann <tiago.herrmann@canonical.com>
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

#include "callmanager.h"
#include "callentry.h"
#include "telepathyhelper.h"
#include "accountentry.h"

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/PendingContacts>
#include <QDBusInterface>

typedef QMap<QString, QVariant> dbusQMap;
Q_DECLARE_METATYPE(dbusQMap)

CallManager *CallManager::instance()
{
    static CallManager *self = new CallManager();
    return self;
}

CallManager::CallManager(QObject *parent)
: QObject(parent), mNeedsUpdate(false), mConferenceCall(0)
{
    connect(TelepathyHelper::instance(), SIGNAL(channelObserverUnregistered()), SLOT(onChannelObserverUnregistered()));
    connect(this, SIGNAL(hasCallsChanged()), SIGNAL(callsChanged()));
    connect(this, &CallManager::hasCallsChanged, [this] {
        Q_EMIT this->callIndicatorVisibleChanged(this->callIndicatorVisible());
    });

    refreshProperties();

    // connect the dbus signal
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.connect("com.lomiri.TelephonyServiceHandler",
                       "/com/lomiri/TelephonyServiceHandler",
                       "com.lomiri.TelephonyServiceHandler",
                       "CallIndicatorVisibleChanged",
                       this, SLOT(onCallIndicatorVisibleChanged(bool)));
    connection.connect("com.lomiri.TelephonyServiceHandler",
                       "/com/lomiri/TelephonyServiceHandler",
                       "com.lomiri.TelephonyServiceHandler",
                       "ConferenceCallRequestFinished",
                       this, SLOT(onConferenceCallRequestFinished(bool)));
}

void CallManager::refreshProperties()
{
    QDBusInterface handlerPropertiesInterface("com.lomiri.TelephonyServiceHandler",
                                              "/com/lomiri/TelephonyServiceHandler",
                                              "org.freedesktop.DBus.Properties");
    QDBusReply<QVariantMap> reply = handlerPropertiesInterface.call("GetAll",
                                                                    "com.lomiri.TelephonyServiceHandler");
    if (!reply.isValid()) {
        qWarning() << "Failed to refresh the properties from the handler";
        return;
    }

    QVariantMap map = reply.value();
    mCallIndicatorVisible = map["CallIndicatorVisible"].toBool();
    Q_EMIT callIndicatorVisibleChanged(mCallIndicatorVisible);
}

void CallManager::setDBusProperty(const QString &name, const QVariant &value)
{
    QDBusInterface handlerPropertiesInterface("com.lomiri.TelephonyServiceHandler",
                                              "/com/lomiri/TelephonyServiceHandler",
                                              "org.freedesktop.DBus.Properties");
    handlerPropertiesInterface.call("Set",
                                    "com.lomiri.TelephonyServiceHandler",
                                    name, QVariant::fromValue(QDBusVariant(value)));
}

QList<CallEntry *> CallManager::takeCalls(const QList<Tp::ChannelPtr> callChannels)
{
    qDebug() << __PRETTY_FUNCTION__;
    QList<CallEntry*> entries;

    // run through the current calls and check which ones we find
    Q_FOREACH(CallEntry *entry, mCallEntries) {
        if (callChannels.contains(entry->channel())) {
            mCallEntries.removeAll(entry);
            entries << entry;
            entry->disconnect(this);
        }
    }

    // FIXME: check which of those signals we really need to emit here
    Q_EMIT hasCallsChanged();
    Q_EMIT hasBackgroundCallChanged();
    Q_EMIT foregroundCallChanged();
    Q_EMIT backgroundCallChanged();

    return entries;
}

void CallManager::addCalls(const QList<CallEntry *> entries)
{
    Q_FOREACH (CallEntry *entry, entries) {
        if (!mCallEntries.contains(entry)) {
            mCallEntries << entry;
        }
        setupCallEntry(entry);
    }

    // FIXME: check which of those signals we really need to emit here
    Q_EMIT hasCallsChanged();
    Q_EMIT hasBackgroundCallChanged();
    Q_EMIT foregroundCallChanged();
    Q_EMIT backgroundCallChanged();
}

bool CallManager::callIndicatorVisible() const
{
    return hasCalls() && mCallIndicatorVisible;
}

void CallManager::setCallIndicatorVisible(bool visible)
{
    setDBusProperty("CallIndicatorVisible", visible);
}

void CallManager::setupCallEntry(CallEntry *entry)
{
    connect(entry,
            SIGNAL(callEnded()),
            SLOT(onCallEnded()));
    connect(entry,
            SIGNAL(heldChanged()),
            SIGNAL(foregroundCallChanged()));
    connect(entry,
            SIGNAL(activeChanged()),
            SIGNAL(foregroundCallChanged()));
    connect(entry,
            SIGNAL(heldChanged()),
            SIGNAL(backgroundCallChanged()));
    connect(entry,
            SIGNAL(activeChanged()),
            SIGNAL(hasBackgroundCallChanged()));
    connect(entry,
            SIGNAL(activeChanged()),
            SIGNAL(hasCallsChanged()));
}

void CallManager::onChannelObserverUnregistered()
{
    // do not clear the manager right now, wait until the observer is re-registered
    // to avoid flickering in the UI
    mNeedsUpdate = true;
}

void CallManager::startCall(const QString &phoneNumber, const QString &accountId)
{
    AccountEntry *account;
    if (accountId.isNull()) {
        account = TelepathyHelper::instance()->defaultCallAccount();
        if (!account) {
            account = TelepathyHelper::instance()->accounts()[0];
        }
    } else {
        account = TelepathyHelper::instance()->accountForId(accountId);
    }

    if (!account) {
        return;
    }

    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    phoneAppHandler->call("StartCall", phoneNumber, account->accountId());
}

void CallManager::onCallIndicatorVisibleChanged(bool visible)
{
    mCallIndicatorVisible = visible;
    Q_EMIT callIndicatorVisibleChanged(visible);
}

void CallManager::onConferenceCallRequestFinished(bool succeeded)
{
    if (!succeeded) {
        Q_EMIT conferenceRequestFailed();
    }
}

CallEntry *CallManager::foregroundCall() const
{
    CallEntry *call = 0;

    // if we have only one call, return it as being always in foreground
    // even if it is held
    QList<CallEntry*> calls = activeCalls();
    if (calls.count() == 1) {
        call = calls.first();
    } else {
        Q_FOREACH(CallEntry *entry, calls) {
            if (!entry->isHeld()) {
                call = entry;
                break;
            }
        }
    }

    return call;
}

CallEntry *CallManager::backgroundCall() const
{
    QList<CallEntry*> calls = activeCalls();
    // if we have only one call, assume there is no call in background
    // even if the foreground call is held
    if (calls.count() == 1) {
        return 0;
    }

    Q_FOREACH(CallEntry *entry, calls) {
        if (entry->isHeld()) {
            return entry;
        }
    }

    return 0;
}

QList<CallEntry *> CallManager::activeCalls() const
{
    QList<CallEntry*> calls;
    if (mConferenceCall) {
        calls << mConferenceCall;
    }

    Q_FOREACH(CallEntry *entry, mCallEntries) {
        if (entry->isActive() || entry->dialing()) {
            calls << entry;
        }
    }

    return calls;
}

QQmlListProperty<CallEntry> CallManager::calls()
{
    return QQmlListProperty<CallEntry>(this, 0, callsCount, callAt);
}

bool CallManager::hasCalls() const
{
    // check if the callmanager already has active calls
    if (activeCalls().count() > 0) {
        return true;
    }

    // if that's not the case, and if not in greeter mode, query the telephony-service-handler
    // for the availability of calls.
    // this is done only to get the live call view on clients as soon as possible, even before the
    // telepathy observer is configured
    // Also, we have to avoid creating instances of GreeterContacts here to query if we are in greeter mode,
    // otherwise we might end up with a deadlock: unity -> telephony-service -> unity
    if (qgetenv("XDG_SESSION_CLASS") != "greeter") {
        QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
        QDBusReply<bool> reply = phoneAppHandler->call("HasCalls");
        if (reply.isValid()) {
            return reply.value();
        }
    }

    return false;
}

bool CallManager::hasBackgroundCall() const
{
    return activeCalls().count() > 1;
}

int CallManager::callsCount(QQmlListProperty<CallEntry> *p)
{
    return CallManager::instance()->activeCalls().count();
}

CallEntry *CallManager::callAt(QQmlListProperty<CallEntry> *p, int index)
{
    return CallManager::instance()->activeCalls()[index];
}

void CallManager::onCallChannelAvailable(Tp::CallChannelPtr channel)
{
    // if this is the first call after re-registering the observer, clear the data
    if (mNeedsUpdate) {
        Q_FOREACH(CallEntry *entry, mCallEntries) {
            entry->deleteLater();
        }
        mCallEntries.clear();
        if (mConferenceCall) {
            mConferenceCall->deleteLater();
            mConferenceCall = 0;
        }
        mNeedsUpdate = false;
    }

    CallEntry *entry = new CallEntry(channel, this);
    if (entry->isConference()) {
        // assume there can be only one conference call at any time for now
        mConferenceCall = entry;

        // check if any of the existing channels belong to the conference
        // if they do, move them to the conference
        QList<CallEntry*> entries = takeCalls(channel->conferenceChannels());
        Q_FOREACH(CallEntry *entry, entries) {
            mConferenceCall->addCall(entry);
        }
        setupCallEntry(mConferenceCall);
    } else if (mConferenceCall && mConferenceCall->channel()->conferenceChannels().contains(channel)){
        // if the call channel belongs to the conference, don't add it here, move it to the conference itself
        mConferenceCall->addCall(entry);
    } else {
        mCallEntries.append(entry);
        setupCallEntry(entry);
    }

    // FIXME: check which of those signals we really need to emit here
    Q_EMIT hasCallsChanged();
    Q_EMIT hasBackgroundCallChanged();
    Q_EMIT foregroundCallChanged();
    Q_EMIT backgroundCallChanged();
}

void CallManager::onCallEnded()
{
    qDebug() << __PRETTY_FUNCTION__;
    // FIXME: handle multiple calls
    CallEntry *entry = qobject_cast<CallEntry*>(sender());
    if (!entry) {
        return;
    }

    // at this point the entry should be removed
    if (entry == mConferenceCall) {
        mConferenceCall = 0;
    } else {
        mCallEntries.removeAll(entry);
    }

    Q_EMIT callEnded(entry);
    Q_EMIT hasCallsChanged();
    Q_EMIT hasBackgroundCallChanged();
    Q_EMIT foregroundCallChanged();
    Q_EMIT backgroundCallChanged();
    entry->deleteLater();
}

void CallManager::mergeCalls(CallEntry *firstCall, CallEntry *secondCall)
{
    QDBusInterface *handlerInterface = TelepathyHelper::instance()->handlerInterface();
    // if there is already a conference call, just merge the remaining channels
    // in the existing conference
    if (firstCall->isConference() || secondCall->isConference()) {
        CallEntry *conferenceCall = firstCall->isConference() ? firstCall : secondCall;
        CallEntry *otherCall = firstCall->isConference() ? secondCall : firstCall;
        handlerInterface->call("MergeCall", conferenceCall->channel()->objectPath(), otherCall->channel()->objectPath());
    } else {
        handlerInterface->call("CreateConferenceCall", QStringList() << firstCall->channel()->objectPath()
                                                                     << secondCall->channel()->objectPath());
    }
}

void CallManager::splitCall(CallEntry *callEntry)
{
    QDBusInterface *handlerInterface = TelepathyHelper::instance()->handlerInterface();
    handlerInterface->call("SplitCall", callEntry->channel()->objectPath());
}

void CallManager::playTone(const QString &key)
{
    QDBusInterface *phoneAppHandler = TelepathyHelper::instance()->handlerInterface();
    /* calling without channel, DTMF tone is played only locally */
    phoneAppHandler->call("SendDTMF", "" , key);
}

bool CallManager::handleMediaKey(bool doubleClick)
{
    QDBusInterface *approverInterface = TelepathyHelper::instance()->approverInterface();
    QDBusReply<bool> reply = approverInterface->call("HandleMediaKey", doubleClick);
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}

