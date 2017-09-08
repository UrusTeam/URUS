
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#include "CORE_URUS_NAMESPACE.h"

#include "CoreUrusRCOutput.h"

namespace NSCORE_URUS {

CLCoreUrusRCOutput::CLCoreUrusRCOutput()
{}

void CLCoreUrusRCOutput::init()
{}

void CLCoreUrusRCOutput::set_freq(uint32_t chmask, uint16_t freq_hz)
{}

uint16_t CLCoreUrusRCOutput::get_freq(uint8_t ch)
{
    return 50;
}

void CLCoreUrusRCOutput::enable_ch(uint8_t ch)
{}

void CLCoreUrusRCOutput::disable_ch(uint8_t ch)
{}

void CLCoreUrusRCOutput::write(uint8_t ch, uint16_t period_us)
{}

uint16_t CLCoreUrusRCOutput::read(uint8_t ch)
{
    return 900;
}

void CLCoreUrusRCOutput::read(uint16_t* period_us, uint8_t len)
{}

} //end namespace NSCORE_URUS