#pragma once

#include <AP_Common/AP_Common.h>

// Global parameter class.
//
class Parameters {
public:

    /*
     *  The value of k_format_version determines whether the existing
     *  eeprom data is considered valid. You should only change this
     *  value under the following circumstances:
     *
     *  1) the meaning of an existing eeprom parameter changes
     *
     *  2) the value of an existing k_param_* enum value changes
     *
     *  Adding a new parameter should _not_ require a change to
     *  k_format_version except under special circumstances. If you
     *  change it anyway then all ArduPlane users will need to reload all
     *  their parameters. We want that to be an extremely rare
     *  thing. Please do not just change it "just in case".
     *
     *  To determine if a k_param_* value has changed, use the rules of
     *  C++ enums to work out the value of the neighboring enum
     *  values. If you don't know the C++ enum rules then please ask for
     *  help.
     */

    //////////////////////////////////////////////////////////////////
    // STOP!!! DO NOT CHANGE THIS VALUE UNTIL YOU FULLY UNDERSTAND THE
    // COMMENTS ABOVE. IF UNSURE, ASK ANOTHER DEVELOPER!!!
    static const uint16_t k_format_version = 3;
    //////////////////////////////////////////////////////////////////


    // The parameter software_type is set up solely for ground station use
    // and identifies the software type (eg ArduPilotMega versus ArduCopterMega)
    // GCS will interpret values 0-9 as ArduPilotMega.  Developers may use
    // values within that range to identify different branches.
    //
    static const uint16_t k_software_type = 10;

    enum {
        // Layout version number, always key zero.
        //
        k_param_format_version = 0,
        k_param_software_type,

        k_param_rc_1,
        k_param_rc_2,
        k_param_rc_3,
        k_param_rc_4,
        k_param_rc_5,
        k_param_rc_6,
        k_param_rc_7,
        k_param_rc_8,
        k_param_rc_9,
        k_param_rc_10,

        k_param_rc_1B,
        k_param_rc_2B,
        k_param_rc_3B,
        k_param_rc_4B,
        k_param_rc_5B,
        k_param_rc_6B,
        k_param_rc_7B,
        k_param_rc_8B,
        k_param_rc_9B,
        k_param_rc_10B,
        //
        // 220: Waypoint data
        //
        k_param_command_total,

        // 254,255: reserved
    };

    AP_Int16 format_version;
    AP_Int8 software_type;

    RC_Channel rc_1;
    RC_Channel rc_2;
    RC_Channel rc_3;
    RC_Channel rc_4;
    RC_Channel rc_5;
    RC_Channel rc_6;
    RC_Channel rc_7;
    RC_Channel rc_8;
    RC_Channel rc_9;
    RC_Channel rc_10;

    RC_Channel rc_1B;
    RC_Channel rc_2B;
    RC_Channel rc_3B;
    RC_Channel rc_4B;
    RC_Channel rc_5B;
    RC_Channel rc_6B;
    RC_Channel rc_7B;
    RC_Channel rc_8B;
    RC_Channel rc_9B;
    RC_Channel rc_10B;

    AP_Int8 command_total; // 1 if HOME is set

    Parameters()
        {}
};

extern const AP_Param::Info var_info[];
