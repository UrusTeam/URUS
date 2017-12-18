#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include <avr/io.h>
#include <avr/eeprom.h>

#include "CoreUrusStorage_Avr.h"

void CLCoreUrusEEStorage_Avr::read_block(void *dst, uint16_t src, size_t n)
{
    eeprom_read_block(dst,(const void*)src,n);
}

void CLCoreUrusEEStorage_Avr::write_block(uint16_t dst, const void *src, size_t n)
{
    uint8_t *p = (uint8_t *)src;
    while (n--) {
        /*
          it is much faster to read than write, so it is worth
          checking if the value is already correct
         */
        uint8_t b = eeprom_read_byte((uint8_t*)dst);
        if (b != *p) {
            eeprom_write_byte((uint8_t*)dst, *p);
        }
        dst++;
        p++;
    }
}

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
