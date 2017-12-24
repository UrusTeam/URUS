#include "GCS_Mavlink.h"

#if (CONFIG_SHAL_CORE == SHAL_CORE_APM) || (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

GCS_Dummy _gcs;

#if defined(SHAL_CORE_APM2)
void (*GCS_MAVLINK::msg_snoop)(const mavlink_message_t* msg) = nullptr;

GCS *GCS::_singleton = nullptr;

GCS &gcs()
{
    return *GCS::instance();
}
#endif

const AP_Param::GroupInfo GCS_MAVLINK::var_info[] = {
    AP_GROUPEND
};

/*
void GCS_MAVLINK::send_parameter_value_all(const char *param_name, ap_var_type param_type, float param_value)
{}
*/
/*
  send a text message to all GCS
 */

#if defined(SHAL_CORE_APM2)
void GCS::send_text(MAV_SEVERITY severity, const char *fmt, ...)
{}

void GCS_MAVLINK::send_text(MAV_SEVERITY severity, const char *fmt, ...)
{}

void GCS_MAVLINK::send_accelcal_vehicle_position(uint32_t position)
{}

uint8_t GCS_MAVLINK::packet_overhead_chan(mavlink_channel_t chan)
{
    return 0;
}

uint8_t GCS_MAVLINK::mavlink_active = 0;
#endif
#endif
