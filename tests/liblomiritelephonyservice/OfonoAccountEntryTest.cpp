/*
 * Copyright (C) 2015-2017 Canonical, Ltd.
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

#include <QtCore/QObject>
#include <QtTest/QtTest>
#include "telepathytest.h"
#include "ofonoaccountentry.h"
#include "accountentryfactory.h"
#include "mockcontroller.h"

class OfonoAccountEntryTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testAccountType();
    void testConnected();
    void testCompareIds_data();
    void testCompareIds();
    void testEmergencyNumbers();
    void testCountryCode();
    void testSerial();
    void testVoicemailIndicator();
    void testVoicemailNumber();
    void testVoicemailCount();
    void testSimLocked();
    void testEmergencyCallsAvailable_data();
    void testEmergencyCallsAvailable();
    void testNetworkName();
    void testAddressableVCardFields();

private:
    OfonoAccountEntry *mAccount;
    Tp::AccountPtr mTpAccount;
    MockController *mMockController;
};

void OfonoAccountEntryTest::initTestCase()
{
    initialize();
}

void OfonoAccountEntryTest::init()
{
    mTpAccount = addAccount("mock", "ofono", "phone account");
    QVERIFY(!mTpAccount.isNull());
    mAccount = qobject_cast<OfonoAccountEntry*>(AccountEntryFactory::createEntry(mTpAccount, this));
    QVERIFY(mAccount);
    TRY_VERIFY(mAccount->ready());

    // make sure the connection is available
    TRY_VERIFY(mAccount->connected());

    // and make sure the status and status message are the ones we expect
    TRY_COMPARE(mAccount->status(), QString("available"));

    // create the mock controller
    mMockController = new MockController("ofono", this);
}

void OfonoAccountEntryTest::cleanup()
{
    doCleanup();
    mAccount->deleteLater();
    mMockController->deleteLater();
}

void OfonoAccountEntryTest::testAccountType()
{
    QCOMPARE(mAccount->type(), AccountEntry::PhoneAccount);
}

void OfonoAccountEntryTest::testConnected()
{
    // the mock account is enabled/connected by default, so make sure it is like that
    TRY_VERIFY(mAccount->connected());

    // right now the ofono account connection status behave exactly like the generic class,
    // but as the code path is different, test it again
    QSignalSpy connectedChangedSpy(mAccount, SIGNAL(connectedChanged()));

    // now set the account offline and see if the active flag changes correctly
    mMockController->SetOnline(false);
    TRY_VERIFY(connectedChangedSpy.count() > 0);
    TRY_VERIFY(!mAccount->connected());

    // now re-enable the account and check that the entry is updated
    connectedChangedSpy.clear();
    mMockController->SetOnline(true);
    // because of the way the mock was implemented, sometimes this can return two connectedChanged() signals.
    TRY_VERIFY(connectedChangedSpy.count() > 0);
    TRY_VERIFY(mAccount->connected());
}

void OfonoAccountEntryTest::testCompareIds_data()
{
    QTest::addColumn<QString>("first");
    QTest::addColumn<QString>("second");
    QTest::addColumn<bool>("expectedResult");

    QTest::newRow("identical values") << "1234567" << "1234567" << true;
    QTest::newRow("case difference") << "TestId" << "testid" << false;
    QTest::newRow("phone prefix") << "1234567" << "1231234567" << true;
}

void OfonoAccountEntryTest::testCompareIds()
{
    QFETCH(QString, first);
    QFETCH(QString, second);
    QFETCH(bool, expectedResult);

    QCOMPARE(mAccount->compareIds(first, second), expectedResult);
}

void OfonoAccountEntryTest::testEmergencyNumbers()
{
    // check that the list is not empty at startup
    QTRY_VERIFY(!mAccount->emergencyNumbers().isEmpty());

    QSignalSpy emergencyNumbersChangedSpy(mAccount, SIGNAL(emergencyNumbersChanged()));

    QStringList numbers;
    numbers << "111" << "190" << "911";
    qSort(numbers);
    mMockController->SetEmergencyNumbers(numbers);
    TRY_COMPARE(emergencyNumbersChangedSpy.count(), 1);

    QStringList emergencyNumbers = mAccount->emergencyNumbers();
    qSort(emergencyNumbers);

    QCOMPARE(emergencyNumbers, numbers);
}

void OfonoAccountEntryTest::testCountryCode()
{
    QSignalSpy countryCodeChangedSpy(mAccount, SIGNAL(countryCodeChanged()));

    // check that the countryCode is not empty at startup
    QTRY_VERIFY(!mAccount->countryCode().isEmpty());
    QCOMPARE(mAccount->countryCode(), QString("US"));

    countryCodeChangedSpy.clear();

    QString countryCode("BR");
    mMockController->SetCountryCode("BR");
    TRY_COMPARE(countryCodeChangedSpy.count(), 1);

    QString cc = mAccount->countryCode();

    QCOMPARE(cc, countryCode);
}

void OfonoAccountEntryTest::testSerial()
{
    TRY_COMPARE(mAccount->serial(), mMockController->serial());
}

void OfonoAccountEntryTest::testVoicemailIndicator()
{
    // voicemail indicator is off by default on the mock CM
    QVERIFY(!mAccount->voicemailIndicator());

    QSignalSpy voiceMailIndicatorSpy(mAccount, SIGNAL(voicemailIndicatorChanged()));

    // set to true
    mMockController->SetVoicemailIndicator(true);
    TRY_COMPARE(voiceMailIndicatorSpy.count(), 1);
    QVERIFY(mAccount->voicemailIndicator());

    // and set back to false
    voiceMailIndicatorSpy.clear();
    mMockController->SetVoicemailIndicator(false);
    TRY_VERIFY(!mAccount->voicemailIndicator());
    QVERIFY(voiceMailIndicatorSpy.count() > 0);
}

void OfonoAccountEntryTest::testVoicemailNumber()
{
    // check that the number is not empty at startup
    TRY_VERIFY(!mAccount->voicemailNumber().isEmpty());

    QSignalSpy voicemailNumberSpy(mAccount, SIGNAL(voicemailNumberChanged()));

    // try changing the number
    QString number("12345");
    mMockController->SetVoicemailNumber(number);
    TRY_COMPARE(voicemailNumberSpy.count(), 1);
    QCOMPARE(mAccount->voicemailNumber(), number);
}

void OfonoAccountEntryTest::testVoicemailCount()
{
    QSignalSpy voicemailCountSpy(mAccount, SIGNAL(voicemailCountChanged()));

    // check that the count is zero at startup
    QCOMPARE((int)mAccount->voicemailCount(), 0);

    // set it to a bigger value
    int count = 10;
    mMockController->SetVoicemailCount(count);
    TRY_COMPARE(voicemailCountSpy.count(), 1);
    QCOMPARE((int)mAccount->voicemailCount(), count);

    // and back to zero
    voicemailCountSpy.clear();
    mMockController->SetVoicemailCount(0);
    TRY_COMPARE((int)mAccount->voicemailCount(), 0);
    QVERIFY(voicemailCountSpy.count() > 0);
}

void OfonoAccountEntryTest::testSimLocked()
{
    QSignalSpy simLockedSpy(mAccount, SIGNAL(simLockedChanged()));

    // check that it is not locked by default
    QVERIFY(!mAccount->simLocked());

    // now try to set the status to simlocked
    mMockController->SetPresence("simlocked", "simlocked");
    TRY_COMPARE(simLockedSpy.count(), 1);
    QVERIFY(mAccount->simLocked());
}

void OfonoAccountEntryTest::testEmergencyCallsAvailable_data()
{
    QTest::addColumn<QString>("status");
    QTest::addColumn<bool>("available");

    QTest::newRow("available") << "available" << true;
    QTest::newRow("away") << "away" << true;
    QTest::newRow("simlocked") << "simlocked" << true;
    QTest::newRow("flightmode") << "flightmode" << false;
    QTest::newRow("nosim") << "nosim" << true;
    QTest::newRow("nomodem") << "nomodem" << false;
    QTest::newRow("registered") << "registered" << true;
    QTest::newRow("roaming") << "roaming" << true;
    QTest::newRow("unregistered") << "unregistered" << true;
    QTest::newRow("denied") << "denied" << true;
    QTest::newRow("unknown") << "unknown" << true;
    QTest::newRow("searching") << "searching" << true;
}

void OfonoAccountEntryTest::testEmergencyCallsAvailable()
{
    QFETCH(QString, status);
    QFETCH(bool, available);

    mMockController->SetPresence(status, "");
    TRY_COMPARE(mAccount->status(), status);
    QCOMPARE(mAccount->emergencyCallsAvailable(), available);
}

void OfonoAccountEntryTest::testNetworkName()
{
    QSignalSpy networkNameChangedSpy(mAccount, SIGNAL(statusMessageChanged()));

    // set the value
    QString statusMessage("SomeNetwork");
    mMockController->SetPresence("available", statusMessage);

    TRY_COMPARE(networkNameChangedSpy.count(), 1);
    QCOMPARE(mAccount->networkName(), statusMessage);
}

void OfonoAccountEntryTest::testAddressableVCardFields()
{
    QVERIFY(!mAccount->addressableVCardFields().isEmpty());
    QCOMPARE(mAccount->addressableVCardFields(), mTpAccount->protocolInfo().addressableVCardFields());
}

QTEST_MAIN(OfonoAccountEntryTest)
#include "OfonoAccountEntryTest.moc"
