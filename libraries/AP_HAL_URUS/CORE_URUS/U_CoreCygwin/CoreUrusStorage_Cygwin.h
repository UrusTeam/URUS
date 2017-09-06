#pragma once
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include "../CORE_URUS_NAMESPACE.h"

#include "../CoreUrusStorage.h"

class CLCoreUrusEEStorage_Cygwin : public NSCORE_URUS::CLCoreUrusEEStorage {
public:
    CLCoreUrusEEStorage_Cygwin() {
        _eeprom_fd = -1;
    }
    void init() {}
    void read_block(void *dst, uint16_t src, size_t n);
    void write_block(uint16_t dst, const void* src, size_t n);

private:
    int _eeprom_fd;
    void _eeprom_open(void);
};

#endif  // __CYGWIN__