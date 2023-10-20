/*
 * Copyright (C) 2015-2016 Canonical, Ltd.
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

#include <QtCore/QObject>
#include <QtTest/QtTest>

#include "config.h"
#include "protocol.h"

// just to make the constructor public
class TestProtocol : public Protocol
{
    Q_OBJECT
public:
    TestProtocol(const QString &name,
                 Protocol::Features features,
                 const QString &fallbackProtocol,
                 Protocol::MatchRule fallbackMatchRule,
                 const QString &fallbackSourceProperty,
                 const QString &fallbackDestinationProperty,
                 bool showOnSelector,
                 bool showOnlineStatus,
                 const QString &backgroundFile,
                 const QString &icon,
                 const QString &serviceName = QString(),
                 const QString &serviceDisplayName = QString(),
                 bool joinExistingChannels = false,
                 bool returnToSend = false,
                 bool enableAttachments = true,
                 bool enableRejoin = false,
                 bool enableTabCompletion = false,
                 bool leaveRoomsOnClose = false,
                 bool enableChatStates = false,
                 QObject *parent = 0)
     : Protocol(name, features, fallbackProtocol, fallbackMatchRule, fallbackSourceProperty, fallbackDestinationProperty,
                showOnSelector, showOnlineStatus, backgroundFile, icon, serviceName, serviceDisplayName, joinExistingChannels, returnToSend, enableAttachments, enableRejoin, enableTabCompletion, leaveRoomsOnClose, enableChatStates, parent) { }
};

class ProtocolTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testBasicInfo();
    void testFromFile();
};

void ProtocolTest::testBasicInfo()
{
    QString name("foobar");
    Protocol::Features features(Protocol::TextChats);
    QString fallbackProtocol("theFallback");
    Protocol::MatchRule fallbackMatchRule(Protocol::MatchProperties);
    QString fallbackSourceProperty("sourceProperty");
    QString fallbackDestinationProperty("destinationProperty");
    bool showOnSelector(false);
    bool showOnlineStatus(true);
    QString backgroundImage("/tmp/background.png");
    QString icon("/tmp/icon.png");
    QString serviceName("The service");
    QString serviceDisplayName("The service display name");
    bool joinExistingChannels = true;
    bool returnToSend = true;
    bool enableAttachments = false;
    bool enableRejoin = true;
    bool enableTabCompletion = false;
    bool leaveRoomsOnClose = true;
    bool enableChatStates = false;

    TestProtocol protocol(name, features, fallbackProtocol, fallbackMatchRule, fallbackSourceProperty, fallbackDestinationProperty,
                          showOnSelector, showOnlineStatus, backgroundImage, icon, serviceName, serviceDisplayName, joinExistingChannels, returnToSend, enableAttachments, enableRejoin, enableTabCompletion, leaveRoomsOnClose, enableChatStates, this);
    QCOMPARE(protocol.name(), name);
    QCOMPARE(protocol.features(), features);
    QCOMPARE(protocol.fallbackProtocol(), fallbackProtocol);
    QCOMPARE(protocol.fallbackMatchRule(), fallbackMatchRule);
    QCOMPARE(protocol.fallbackSourceProperty(), fallbackSourceProperty);
    QCOMPARE(protocol.fallbackDestinationProperty(), fallbackDestinationProperty);
    QCOMPARE(protocol.showOnSelector(), showOnSelector);
    QCOMPARE(protocol.showOnlineStatus(), showOnlineStatus);
    QCOMPARE(protocol.backgroundImage(), backgroundImage);
    QCOMPARE(protocol.icon(), icon);
    QCOMPARE(protocol.serviceName(), serviceName);
    QCOMPARE(protocol.serviceDisplayName(), serviceDisplayName);
    QCOMPARE(protocol.joinExistingChannels(), joinExistingChannels);
    QCOMPARE(protocol.returnToSend(), returnToSend);
    QCOMPARE(protocol.enableAttachments(), enableAttachments);
    QCOMPARE(protocol.enableRejoin(), enableRejoin);
    QCOMPARE(protocol.enableTabCompletion(), enableTabCompletion);
    QCOMPARE(protocol.leaveRoomsOnClose(), leaveRoomsOnClose);
    QCOMPARE(protocol.enableChatStates(), enableChatStates);
    QCOMPARE(protocol.parent(), this);
}

void ProtocolTest::testFromFile()
{
    // check that calling fromFile() on an invalid path returns 0
    Protocol *nullProtocol = Protocol::fromFile("/non/existent/path/to/a/dummy.protocol");
    QVERIFY(!nullProtocol);

    // and now check a valid protocol
    Protocol *protocol = Protocol::fromFile(protocolsDir() + "/foo.protocol");
    QVERIFY(protocol);
    QCOMPARE(protocol->name(), QString("foo"));
    QCOMPARE(protocol->features(), Protocol::Features(Protocol::TextChats | Protocol::VoiceCalls));
    QCOMPARE(protocol->fallbackProtocol(), QString("bar"));
    QCOMPARE(protocol->fallbackMatchRule(), Protocol::MatchProperties);
    QCOMPARE(protocol->fallbackSourceProperty(), QString("theSourceProperty"));
    QCOMPARE(protocol->fallbackDestinationProperty(), QString("theDestinationProperty"));
    QCOMPARE(protocol->showOnSelector(), false);
    QCOMPARE(protocol->showOnlineStatus(), true);
    QCOMPARE(protocol->backgroundImage(), QString("/tmp/background.png"));
    QCOMPARE(protocol->icon(), QString("/tmp/icon.png"));
    QCOMPARE(protocol->serviceName(), QString("The Service"));
    QCOMPARE(protocol->serviceDisplayName(), QString("The Service Display Name"));
    QCOMPARE(protocol->joinExistingChannels(), true);
    QCOMPARE(protocol->returnToSend(), false);
    QCOMPARE(protocol->enableAttachments(), false);
    QCOMPARE(protocol->enableRejoin(), true);
    QCOMPARE(protocol->enableTabCompletion(), true);
    QCOMPARE(protocol->leaveRoomsOnClose(), true);
    QCOMPARE(protocol->enableChatStates(), true);
}

QTEST_MAIN(ProtocolTest)
#include "ProtocolTest.moc"
