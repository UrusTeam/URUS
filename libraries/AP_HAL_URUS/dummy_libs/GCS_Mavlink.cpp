#include "GCS_Mavlink.h"

void GCS_MAVLINK::send_parameter_value_all(char const*, ap_var_type, float)
{
    return;
}

void GCS_MAVLINK::packetReceived(__mavlink_status const&, __mavlink_message&)
{
    
}
