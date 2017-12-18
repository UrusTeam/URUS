#pragma once
#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS_NAMESPACE.h"

#include "../CoreUrusStorage.h"

class CLCoreUrusEEStorage_Avr : public NSCORE_URUS::CLCoreUrusEEStorage {
public:
    CLCoreUrusEEStorage_Avr()
    {}

    void init() override {}
    void read_block(void *dst, uint16_t src, size_t n) override;
    void write_block(uint16_t dst, const void* src, size_t n) override;

};

#endif  // CONFIG_SHAL_CORE == SHAL_CORE_APM
