#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_APM)

#include "../CORE_URUS_NAMESPACE.h"
#include "../CoreUrusUtil.h"

#include "CoreUrusUtil_Avr.h"

#include <stdlib.h>
#include <stdint.h>

static const uint32_t *stack_low;
extern unsigned __brkval;

#define STACK_OFFSET 2
#define SENTINEL 0x28021967

/*
 *  return the current stack pointer
 */
static __attribute__((noinline)) const uint32_t *current_stackptr(void)
{
    return (const uint32_t *)__builtin_frame_address(0);
}

/*
 *  this can be added in deeply nested code to ensure we catch
 *  deep stack usage. It should be caught by the sentinel, but this
 *  is an added protection
 */
static void memcheck_update_stackptr(void)
{
    if (current_stackptr() < stack_low) {
        uintptr_t s = (uintptr_t)(current_stackptr() - STACK_OFFSET);
        stack_low = (uint32_t *)(s & ~3);
    }
}

/*
 *  this returns the real amount of free memory by looking for
 *  overwrites of the stack sentinel values
 */
unsigned memcheck_available_memory(void)
{
    memcheck_update_stackptr();
    while (*stack_low != SENTINEL && stack_low > (const uint32_t *)__brkval) {
        stack_low--;
    }
    return (uintptr_t)(stack_low) - __brkval;
}

CLCoreUrusUtil_Avr::CLCoreUrusUtil_Avr() :
    NSCORE_URUS::CLCoreUrusUtil()
{}

/*
 *  initialise memcheck, setting up the sentinels
 */
void CLCoreUrusUtil_Avr::memcheck_init(void)
{
    uint32_t *p;
    free(malloc(1)); // ensure heap is initialised
    stack_low = current_stackptr();
    memcheck_update_stackptr();
    for (p=(uint32_t *)(stack_low-1); p>(uint32_t *)__brkval; p--) {
        *p = SENTINEL;
    }
}

uint32_t CLCoreUrusUtil_Avr::available_memory(void)
{
    return memcheck_available_memory();
}

#endif // CONFIG_SHAL_CORE == SHAL_CORE_APM
