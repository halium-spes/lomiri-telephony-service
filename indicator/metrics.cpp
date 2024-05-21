/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Gustavo Pichorim Boiko <gustavo.boiko@canonical.com>
 *  Ugo Riboni <ugo.riboni@canonical.com>
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

#include "metrics.h"
#include <QDebug>

const QString APP_ID = QString("lomiri-telephony-service");
const QString MESSAGES_RECEIVED_STATISTICS_ID = QString("text-messages-received");
const QString MESSAGES_SENT_STATISTICS_ID = QString("text-messages-sent");
const QString DIALER_INCOMING_STATISTICS_ID = QString("dialer-calls-incoming");
const QString DIALER_OUTGOING_STATISTICS_ID = QString("dialer-calls-outgoing");
const QString DIALER_CALL_DURATION_STATISTICS_ID = QString("dialer-calls-duration");

#define GettextMarkExtraction(x) x

using namespace UserMetricsInput;

Metrics::Metrics(QObject *parent) :
    QObject(parent)
{
    try {
        mMetricManager.reset(MetricManager::getInstance());
        mMetrics[SentMessages] = mMetricManager->add(MetricParameters(MESSAGES_SENT_STATISTICS_ID).formatString(GettextMarkExtraction("<b>%1</b> text messages sent today"))
                                                      .emptyDataString(GettextMarkExtraction("No text messages sent today")).textDomain(APP_ID).minimum(0.0));
        mMetrics[ReceivedMessages] = mMetricManager->add(MetricParameters(MESSAGES_RECEIVED_STATISTICS_ID).formatString(GettextMarkExtraction("<b>%1</b> text messages received today"))
                                                     .emptyDataString(GettextMarkExtraction("No text messages received today")).textDomain(APP_ID).minimum(0.0));
        mMetrics[IncomingCalls] = mMetricManager->add(MetricParameters(DIALER_INCOMING_STATISTICS_ID).formatString(GettextMarkExtraction("<b>%1</b> calls received today"))
                                                      .emptyDataString(GettextMarkExtraction("No calls received today")).textDomain(APP_ID).minimum(0.0));
        mMetrics[OutgoingCalls] = mMetricManager->add(MetricParameters(DIALER_OUTGOING_STATISTICS_ID).formatString(GettextMarkExtraction("<b>%1</b> calls made today"))
                                                      .emptyDataString(GettextMarkExtraction("No calls made today")).textDomain(APP_ID).minimum(0.0));
        mMetrics[CallDurations] = mMetricManager->add(MetricParameters(DIALER_CALL_DURATION_STATISTICS_ID).formatString(GettextMarkExtraction("Spent <b>%1</b> minutes in calls today"))
                                                      .emptyDataString(GettextMarkExtraction("No calls made today")).textDomain(APP_ID).minimum(0.0));
    } catch(std::exception &e) {
        qWarning() << "Error connecting to metrics service:" << e.what();
    }
}

Metrics *Metrics::instance()
{
    static Metrics *self = new Metrics();
    return self;
}

void Metrics::increment(Metrics::MetricType metric, double amount)
{
    MetricPtr metricPtr;
    metricPtr = mMetrics[metric];
    if (!metricPtr) {
        return;
    }
    try {
        metricPtr->increment(amount);
    } catch(std::exception &e) {
        qWarning() << "Error incrementing telephony metric:" << e.what();
    }
}
