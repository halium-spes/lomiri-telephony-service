 /* Copyright (C) 2022 Ubports Foundation
 *
 *
 * This file is part of telepathy-ofono
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

#ifndef CELLBROADCASTCONSTANTS_H
#define CELLBROADCASTCONSTANTS_H

namespace CellBroadcast
{

static const char* CELLBROADCAST_IDENTIFIER = "x-ofono-cellbroadcast";
static const char* CELLBROADCAST_IDENTIFIER_TYPE = "x-ofono-cellbroadcast-type";

/** EU Alert Alert Levels - ref: https://www.etsi.org/deliver/etsi_ts/102900_102999/102900/01.03.01_60/ts_102900v010301p.pdf*/
enum Type {
    TYPE_LEVEL_1 = 1, // CMAS Presidential Alert
    TYPE_LEVEL_2, // CMAS Extreme Alert
    TYPE_LEVEL_3, // CMAS Severe Alert
    TYPE_LEVEL_4, // CMAS Public Safety Alert
    TYPE_INFO, // no equivalent
    TYPE_AMBER, // CMAS Child Abduction Emergency Alert
    TYPE_MONTHLY_TEST,
    TYPE_TEST,
    TYPE_EXERCISE,
    TYPE_RESERVED,
    TYPE_OTHER
};


/** Start of ETWS Message Identifier types */
enum ETWS {
    ETWS_ALERT_EARTHQUAKE = 4352, // ETWS Message Identifier for earthquake warning message.
    ETWS_ALERT_TSUNAMI = 4353, // ETWS Message Identifier for tsunami warning message
    ETWS_ALERT_EARTHQUAKE_AND_TSUNAMI = 4354, // ETWS Message Identifier for earthquake and tsunami combined warning message.
    ETWS_ALERT_TEST = 4355, // ETWS Message Identifier for Test : discarded in ofono
    ETWS_ALERT_OTHER = 4356 // ETWS Message Identifier for messages related to other emergency types.
};

/** Start of CMAS Message Identifier range. */
enum CMAS {
    /** CMAS Message Identifier for Presidential Level alerts. */
    CMAS_ALERT_PRESIDENTIAL_LEVEL = 4370,
    /** CMAS Message Identifier for Extreme alerts, Urgency=Immediate, Certainty=Observed. */
    CMAS_ALERT_EXTREME_IMMEDIATE_OBSERVED = 4371,
    /** CMAS Message Identifier for Extreme alerts, Urgency=Immediate, Certainty=Likely. */
    CMAS_ALERT_EXTREME_IMMEDIATE_LIKELY = 4372,
    /** CMAS Message Identifier for Extreme alerts, Urgency=Expected, Certainty=Observed. */
    CMAS_ALERT_EXTREME_EXPECTED_OBSERVED = 4373,
    /** CMAS Message IdentifCMAS_ALERT_EXTREME_IMMEDIATE_OBSERVEDier for Extreme alerts, Urgency=Expected, Certainty=Likely. */
    CMAS_ALERT_EXTREME_EXPECTED_LIKELY = 4374,
    /** CMAS Message Identifier for Severe alerts, Urgency=Immediate, Certainty=Observed. */
    CMAS_ALERT_SEVERE_IMMEDIATE_OBSERVED = 4375,
    /** CMAS Message Identifier for Severe alerts, Urgency=Immediate, Certainty=Likely. */
    CMAS_ALERT_SEVERE_IMMEDIATE_LIKELY = 4376,
    /** CMAS Message Identifier for Severe alerts, Urgency=Expected, Certainty=Observed. */
    CMAS_ALERT_SEVERE_EXPECTED_OBSERVED = 4377,
    /** CMAS Message Identifier for Severe alerts, Urgency=Expected, Certainty=Likely. */
    CMAS_ALERT_SEVERE_EXPECTED_LIKELY = 4378,
    /** CMAS Message Identifier for Child Abduction Emergency (Amber Alert). */
    CMAS_ALERT_CHILD_ABDUCTION_EMERGENCY = 4379,
    /** CMAS Message Identifier for the Required Monthly Test. */
    CMAS_ALERT_REQUIRED_MONTHLY_TEST = 4380,
    /** CMAS Message Identifier for CMAS Exercise. */
    CMAS_ALERT_EXERCISE = 4381,
    /** CMAS Message Identifier for operator defined use. */
    CMAS_ALERT_OPERATOR_DEFINED_USE = 4382,
    CMAS_ALERT_PRESIDENTIAL_LEVEL_LANGUAGE = 4383,
    /**
     * CMAS Message Identifier for Extreme alerts, Urgency=Immediate, Certainty=Observed
     * for additional languages.
     */
    CMAS_ALERT_EXTREME_IMMEDIATE_OBSERVED_LANGUAGE = 4384,
    /**
     * CMAS Message Identifier for Extreme alerts, Urgency=Immediate, Certainty=Likely
     *  for additional languages.
     */
    CMAS_ALERT_EXTREME_IMMEDIATE_LIKELY_LANGUAGE = 4385,
    /**
     * CMAS Message Identifier for Extreme alerts, Urgency=Expected, Certainty=Observed
     * for additional languages.
     */
    CMAS_ALERT_EXTREME_EXPECTED_OBSERVED_LANGUAGE = 4386,
    /**
     * CMAS Message Identifier for Extreme alerts, Urgency=Expected, Certainty=Likely
     * for additional languages.
     */
    CMAS_ALERT_EXTREME_EXPECTED_LIKELY_LANGUAGE = 4387,
    /**
     * CMAS Message Identifier for Severe alerts, Urgency=Immediate, Certainty=Observed
     * for additional languages.
     */
    CMAS_ALERT_SEVERE_IMMEDIATE_OBSERVED_LANGUAGE = 4388,
    /**
     * CMAS Message Identifier for Severe alerts, Urgency=Immediate, Certainty=Likely
     * for additional languages.
     */
    CMAS_ALERT_SEVERE_IMMEDIATE_LIKELY_LANGUAGE = 4389,
    /**
     * CMAS Message Identifier for Severe alerts, Urgency=Expected, Certainty=Observed
     * for additional languages.
     */
    CMAS_ALERT_SEVERE_EXPECTED_OBSERVED_LANGUAGE = 4390,
    /**
     * CMAS Message Identifier for Severe alerts, Urgency=Expected, Certainty=Likely
     * for additional languages.
     */
    CMAS_ALERT_SEVERE_EXPECTED_LIKELY_LANGUAGE = 4391,
    /**
     * CMAS Message Identifier for Child Abduction Emergency (Amber Alert)
     * for additional languages.
     */
    CMAS_ALERT_CHILD_ABDUCTION_EMERGENCY_LANGUAGE = 4392,

    /** CMAS Message Identifier for the Required Monthly Test  for additional languages. */
    CMAS_ALERT_REQUIRED_MONTHLY_TEST_LANGUAGE = 4393,

    /** CMAS Message Identifier for CMAS Exercise for additional languages. */
    CMAS_ALERT_EXERCISE_LANGUAGE = 4394,

    /** CMAS Message Identifier for operator defined use for additional languages. */
    CMAS_ALERT_OPERATOR_DEFINED_USE_LANGUAGE = 4395,

    /** CMAS Message Identifier for CMAS Public Safety Alerts. */
    CMAS_ALERT_PUBLIC_SAFETY = 4396,

    /** CMAS Message Identifier for CMAS Public Safety Alerts for additional languages. */
    CMAS_ALERT_PUBLIC_SAFETY_LANGUAGE = 4397,

    /** CMAS Message Identifier for CMAS State/Local Test. */
    CMAS_ALERT_STATE_LOCAL_TEST = 4398,

    /** CMAS Message Identifier for CMAS State/Local Test for additional languages. */
    CMAS_ALERT_STATE_LOCAL_TEST_LANGUAGE = 4399,

    /** CMAS Message Identifier for CMAS geo fencing trigger message. */
    CMAS_GEO_FENCING_TRIGGER = 4440
};

}
#endif // CELLBROADCASTCONSTANTS_H

