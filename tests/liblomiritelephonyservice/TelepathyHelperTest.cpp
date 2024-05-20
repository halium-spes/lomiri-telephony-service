/*
 * Copyright (C) 2015 Canonical, Ltd.
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
#include <TelepathyQt/PendingAccount>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/Account>
#include "telepathytest.h"
#include "accountentry.h"
#include "accountentryfactory.h"
#include "telepathyhelper.h"
#include "mockcontroller.h"

Q_DECLARE_METATYPE(AccountEntry*)

class TelepathyHelperTest : public TelepathyTest
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void init();
    void cleanup();
    void testAccounts();
    void testPhoneAccounts();
    void testAccountSorting();
    void testAccountIds();
    void testActiveAccounts();
    void testAccountForId();
    void testAccountForConnection();
    void testEmergencyCallsAvailable();

protected:
    Tp::AccountPtr addAccountAndWait(const QString &manager,
                                     const QString &protocol,
                                     const QString &displayName,
                                     const QVariantMap &parameters = QVariantMap());
private:
    Tp::AccountPtr mGenericTpAccount;
    Tp::AccountPtr mPhoneTpAccount;
    MockController *mGenericController;
    MockController *mPhoneController;
};

void TelepathyHelperTest::initTestCase()
{
    qRegisterMetaType<AccountEntry*>();
    initialize();
    QSignalSpy setupReadySpy(TelepathyHelper::instance(), SIGNAL(setupReady()));
    TRY_COMPARE(setupReadySpy.count(), 1);
}

void TelepathyHelperTest::init()
{
    // add two accounts
    mGenericTpAccount = addAccountAndWait("mock", "mock", "the generic account");
    QVERIFY(!mGenericTpAccount.isNull());

    mPhoneTpAccount = addAccountAndWait("mock", "ofono", "the phone account");
    QVERIFY(!mPhoneTpAccount.isNull());

    // and create the mock controller
    mGenericController = new MockController("mock", this);
    mPhoneController = new MockController("ofono", this);
}

void TelepathyHelperTest::cleanup()
{
    // the accounts are removed in the parent class.
    doCleanup();

    mGenericController->deleteLater();
    mPhoneController->deleteLater();
}

void TelepathyHelperTest::testAccounts()
{
    QCOMPARE(TelepathyHelper::instance()->accounts().count(), 2);
    AccountEntry *first = TelepathyHelper::instance()->accounts()[0];
    AccountEntry *second = TelepathyHelper::instance()->accounts()[1];
    QVERIFY(first->accountId() != second->accountId());

    // now check that new accounts are captured
    QSignalSpy accountsChangedSpy(TelepathyHelper::instance(), SIGNAL(accountsChanged()));
    Tp::AccountPtr newAccount = addAccountAndWait("mock", "mock", "extra");
    QVERIFY(!newAccount.isNull());

    TRY_COMPARE(accountsChangedSpy.count(), 1);
    QCOMPARE(TelepathyHelper::instance()->accounts().count(), 3);

    bool accountFound = false;
    Q_FOREACH(AccountEntry *entry, TelepathyHelper::instance()->accounts()) {
        if (entry->accountId() == newAccount->uniqueIdentifier()) {
            accountFound = true;
            break;
        }
    }
    QVERIFY(accountFound);

    // now remove the extra account and make sure it is properly removed
    accountsChangedSpy.clear();
    QVERIFY(removeAccount(newAccount));
    TRY_COMPARE(accountsChangedSpy.count(), 1);
    QCOMPARE(TelepathyHelper::instance()->accounts().count(), 2);
    QCOMPARE(TelepathyHelper::instance()->accounts()[0]->accountId(), first->accountId());
    QCOMPARE(TelepathyHelper::instance()->accounts()[1]->accountId(), second->accountId());
}

void TelepathyHelperTest::testPhoneAccounts()
{
    QCOMPARE(TelepathyHelper::instance()->phoneAccounts().count(), 1);
    AccountEntry *phoneAccount = TelepathyHelper::instance()->phoneAccounts()[0];
    QVERIFY(phoneAccount->accountId() == mPhoneTpAccount->uniqueIdentifier());

    // now check that new phone accounts are captured
    QSignalSpy phoneAccountsChangedSpy(TelepathyHelper::instance(), SIGNAL(phoneAccountsChanged()));
    Tp::AccountPtr newAccount = addAccount("mock", "ofono", "extra");
    QVERIFY(!newAccount.isNull());

    QTRY_COMPARE(phoneAccountsChangedSpy.count(), 1);
    QCOMPARE(TelepathyHelper::instance()->phoneAccounts().count(), 2);

    bool accountFound = false;
    Q_FOREACH(AccountEntry *entry, TelepathyHelper::instance()->phoneAccounts()) {
        if (entry->accountId() == newAccount->uniqueIdentifier()) {
            accountFound = true;
            break;
        }
    }
    QVERIFY(accountFound);

    // now remove the extra phone account and make sure it is properly removed
    phoneAccountsChangedSpy.clear();
    QVERIFY(removeAccount(newAccount));
    QTRY_COMPARE(phoneAccountsChangedSpy.count(), 1);
    QCOMPARE(TelepathyHelper::instance()->phoneAccounts().count(), 1);
    QCOMPARE(TelepathyHelper::instance()->phoneAccounts()[0]->accountId(), phoneAccount->accountId());
}

void TelepathyHelperTest::testAccountSorting()
{
    // create two accounts with modem-objpath parameters and make sure they are listed first
    QVariantMap parameters;
    parameters["modem-objpath"] = "/phonesim1";
    Tp::AccountPtr firstAccount = addAccountAndWait("mock", "ofono", "firstPhoneAccount", parameters);
    QVERIFY(!firstAccount.isNull());

    parameters["modem-objpath"] = "/phonesim2";
    Tp::AccountPtr secondAccount = addAccountAndWait("mock", "ofono", "secondPhoneAccount", parameters);
    QVERIFY(!secondAccount.isNull());

    // wait for the accounts to appear;
    TRY_COMPARE(TelepathyHelper::instance()->accounts().count(), 4);

    // and check the order
    QCOMPARE(TelepathyHelper::instance()->accounts()[0]->accountId(), firstAccount->uniqueIdentifier());
    QCOMPARE(TelepathyHelper::instance()->accounts()[1]->accountId(), secondAccount->uniqueIdentifier());

    // now add a third account that should go before the two others
    parameters["modem-objpath"] = "/phonesim0";
    Tp::AccountPtr thirdAccount = addAccountAndWait("mock", "ofono", "thirdPhoneAccount", parameters);
    QVERIFY(!thirdAccount.isNull());

    // wait for the accounts to appear;
    TRY_COMPARE(TelepathyHelper::instance()->accounts().count(), 5);
    QCOMPARE(TelepathyHelper::instance()->accounts()[0]->accountId(), thirdAccount->uniqueIdentifier());

    // and remove the created accounts
    QVERIFY(removeAccount(firstAccount));
    QVERIFY(removeAccount(secondAccount));
    QVERIFY(removeAccount(thirdAccount));
}

void TelepathyHelperTest::testAccountIds()
{
    QCOMPARE(TelepathyHelper::instance()->accountIds().count(), 2);

    // now check that new accounts are captured
    QSignalSpy accountIdsChangedSpy(TelepathyHelper::instance(), SIGNAL(accountIdsChanged()));
    Tp::AccountPtr newAccount = addAccountAndWait("mock", "mock", "extra");
    QVERIFY(!newAccount.isNull());

    TRY_COMPARE(accountIdsChangedSpy.count(), 1);
    QCOMPARE(TelepathyHelper::instance()->accountIds().count(), 3);

    // just to make sure check that each account id matches one account
    for (int i = 0; i < 3; ++i) {
        QCOMPARE(TelepathyHelper::instance()->accountIds()[i], TelepathyHelper::instance()->accounts()[i]->accountId());
    }

    // now remove the extra account and make sure it is properly removed
    accountIdsChangedSpy.clear();
    QVERIFY(removeAccount(newAccount));
    TRY_COMPARE(accountIdsChangedSpy.count(), 1);
    QCOMPARE(TelepathyHelper::instance()->accountIds().count(), 2);
    QCOMPARE(TelepathyHelper::instance()->accountIds()[0], TelepathyHelper::instance()->accounts()[0]->accountId());
    QCOMPARE(TelepathyHelper::instance()->accountIds()[1], TelepathyHelper::instance()->accounts()[1]->accountId());
}

void TelepathyHelperTest::testActiveAccounts()
{
    QSignalSpy activeAccountsSpy(TelepathyHelper::instance(), SIGNAL(activeAccountsChanged()));

    // at startup, all accounts are active, so make sure we got two active accounts
    QCOMPARE(TelepathyHelper::instance()->activeAccounts().count(), 2);

    // now set one of the accounts as offline and make sure it is captured
    mGenericController->SetOnline(false);
    TRY_VERIFY(activeAccountsSpy.count() >= 1);
    QTRY_COMPARE(TelepathyHelper::instance()->activeAccounts().count(), 1);
    QCOMPARE(TelepathyHelper::instance()->activeAccounts()[0]->accountId(), mPhoneTpAccount->uniqueIdentifier());

    // set the other account offline to make sure
    activeAccountsSpy.clear();
    mPhoneController->SetOnline(false);
    TRY_VERIFY(activeAccountsSpy.count() >= 1);
    QVERIFY(TelepathyHelper::instance()->activeAccounts().isEmpty());

    // and set both accounts online again
    activeAccountsSpy.clear();
    mGenericController->SetOnline(true);
    mPhoneController->SetOnline(true);
    TRY_VERIFY(activeAccountsSpy.count() >= 2);
    QCOMPARE(TelepathyHelper::instance()->activeAccounts().count(), 2);
}

void TelepathyHelperTest::testAccountForId()
{
    AccountEntry *genericAccount = TelepathyHelper::instance()->accountForId(mGenericTpAccount->uniqueIdentifier());
    QVERIFY(genericAccount);
    QCOMPARE(genericAccount->accountId(), mGenericTpAccount->uniqueIdentifier());

    AccountEntry *phoneAccount = TelepathyHelper::instance()->accountForId(mPhoneTpAccount->uniqueIdentifier());
    QVERIFY(phoneAccount);
    QCOMPARE(phoneAccount->accountId(), mPhoneTpAccount->uniqueIdentifier());
}

void TelepathyHelperTest::testAccountForConnection()
{
    Q_FOREACH(AccountEntry *account, TelepathyHelper::instance()->accounts()) {
        AccountEntry *entry = TelepathyHelper::instance()->accountForConnection(account->account()->connection());
        QVERIFY(entry);
        QCOMPARE(entry, account);
    }
}

void TelepathyHelperTest::testEmergencyCallsAvailable()
{
    QSignalSpy emergencyCallsSpy(TelepathyHelper::instance(), SIGNAL(emergencyCallsAvailableChanged()));

    // check that calls are available by default
    TRY_VERIFY(TelepathyHelper::instance()->emergencyCallsAvailable());

    // set the generic account as "flightmode" and make sure it doesn't affect the emergencyCallsAvailable
    mGenericController->SetPresence("flightmode", "");
    QTest::qWait(500);
    QCOMPARE(emergencyCallsSpy.count(), 0);
    QVERIFY(TelepathyHelper::instance()->emergencyCallsAvailable());

    // now set the phone account as "flightmode", and see if the emergencyCallsAvailable value
    mPhoneController->SetPresence("flightmode", "");
    TRY_VERIFY(emergencyCallsSpy.count() > 0);
    QVERIFY(!TelepathyHelper::instance()->emergencyCallsAvailable());

    // set the generic account online and check if it affects the value
    emergencyCallsSpy.clear();
    mGenericController->SetOnline(true);
    QTest::qWait(500);
    QCOMPARE(emergencyCallsSpy.count(), 0);
    QVERIFY(!TelepathyHelper::instance()->emergencyCallsAvailable());

    // and finally set the phone account back online
    mPhoneController->SetOnline(true);
    TRY_VERIFY(emergencyCallsSpy.count() > 0);
    QVERIFY(TelepathyHelper::instance()->emergencyCallsAvailable());
}

Tp::AccountPtr TelepathyHelperTest::addAccountAndWait(const QString &manager, const QString &protocol, const QString &displayName, const QVariantMap &parameters)
{
    QSignalSpy accountAddedSpy(TelepathyHelper::instance(), SIGNAL(accountAdded(AccountEntry*)));
    Tp::AccountPtr account = addAccount(manager, protocol, displayName, parameters);

    WAIT_FOR(accountAddedSpy.count() == 1);

    AccountEntry *accountEntry = accountAddedSpy.first().first().value<AccountEntry*>();
    WAIT_FOR(accountEntry->ready());

    // and make sure the status and status message are the ones we expect
    WAIT_FOR(accountEntry->status() == QString("available"));

    return account;
}

QTEST_MAIN(TelepathyHelperTest)
#include "TelepathyHelperTest.moc"
