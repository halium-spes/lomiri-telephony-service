/*
 * Copyright (C) 2012-2013 Canonical, Ltd.
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

#include <libintl.h>
#include <libnotify/notify.h>

#include "applicationutils.h"
#include "callchannelobserver.h"
#include "displaynamesettings.h"
#include "indicatordbus.h"
#include "metrics.h"
#include "telepathyhelper.h"
#include "textchannelobserver.h"
#include "voicemailindicator.h"
#include "ussdindicator.h"
#include "authhandler.h"
#include <QCoreApplication>
#include <TelepathyQt/ClientRegistrar>
#include <TelepathyQt/AbstractClient>
#include <TelepathyQt/AccountManager>
#include <TelepathyQt/Contact>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("lomiri-indicator-telephony-service");

    bindtextdomain( "lomiri-telephony-service", "/usr/share/locale" );
    textdomain("lomiri-telephony-service");

    notify_init(gettext("Lomiri Telephony Service Indicator"));

    Tp::registerTypes();

    // check if there is already an instance of the indicator running
    if (ApplicationUtils::checkApplicationRunning(TP_QT_IFACE_CLIENT + ".TelephonyServiceIndicator")) {
        qDebug() << "Found another instance of the indicator. Quitting.";
        return 1;
    }

    // we don't need to call anything on the indicator, it will work by itself
    // make sure we create the voicemail indicator before the telepathy helper
    VoiceMailIndicator voiceMailIndicator;
    Q_UNUSED(voiceMailIndicator);

    // create the dbus object and connect its signals
    USSDIndicator ussdIndicator;
    AuthHandler authHandler;
    IndicatorDBus dbus;
    QObject::connect(&dbus, SIGNAL(clearNotificationsRequested()),
                     &ussdIndicator, SLOT(clear()));
    QObject::connect(&dbus, SIGNAL(clearNotificationsRequested()),
                     &authHandler, SLOT(clear()));

    // register the observer
    QObject::connect(TelepathyHelper::instance(), &TelepathyHelper::setupReady, [&]() {
        TelepathyHelper::instance()->registerChannelObserver("TelephonyServiceIndicator");

        // Connect the textObserver and the callObserver to the channel observer in TelepathyHelper
        CallChannelObserver *callObserver = new CallChannelObserver();
        TextChannelObserver *textObserver = new TextChannelObserver();
        QObject::connect(TelepathyHelper::instance()->channelObserver(), SIGNAL(textChannelAvailable(Tp::TextChannelPtr)),
                         textObserver, SLOT(onTextChannelAvailable(Tp::TextChannelPtr)));
        QObject::connect(TelepathyHelper::instance()->channelObserver(), SIGNAL(callChannelAvailable(Tp::CallChannelPtr)),
                         callObserver, SLOT(onCallChannelAvailable(Tp::CallChannelPtr)));
        QObject::connect(&dbus, SIGNAL(clearNotificationsRequested()),
                         textObserver, SLOT(clearNotifications()));

        // instanciate the display name settings singleton, it will work by itself
        DisplayNameSettings::instance();
    });

    // instanciate the metrics helper
    Metrics::instance();

    dbus.connectToBus();

    return app.exec();
}
