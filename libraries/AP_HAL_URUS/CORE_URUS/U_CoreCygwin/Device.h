/*
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <AP_HAL/HAL.h>
#if (CONFIG_SHAL_CORE == SHAL_CORE_CYGWIN)

#include <inttypes.h>
#include <pthread.h>
#include "CoreUrusScheduler_Cygwin.h"
#include "CoreUrusSemaphores_Cygwin.h"

#define APM_MAIN_PRIORITY_BOOST 241
#define APM_MAIN_PRIORITY       180
#define APM_TIMER_PRIORITY      181
#define APM_SPI_PRIORITY        242
#define APM_CAN_PRIORITY        179
#define APM_I2C_PRIORITY        178
#define APM_UART_PRIORITY        60
#define APM_STORAGE_PRIORITY     59
#define APM_IO_PRIORITY          58
#define APM_SHELL_PRIORITY       57
#define APM_OVERTIME_PRIORITY    10
#define APM_STARTUP_PRIORITY     10

class DeviceBus {
public:
    DeviceBus(uint8_t _thread_priority = APM_I2C_PRIORITY) :
        thread_priority(_thread_priority) {}

    struct DeviceBus *next;
    CLCoreUrusSemaphore_Cygwin semaphore;

    AP_HAL::Device::PeriodicHandle register_periodic_callback(uint32_t period_usec, AP_HAL::Device::PeriodicCb, AP_HAL::Device *hal_device);
    bool adjust_timer(AP_HAL::Device::PeriodicHandle h, uint32_t period_usec);
    static void *bus_thread(void *arg);

private:
    struct callback_info {
        struct callback_info *next;
        AP_HAL::Device::PeriodicCb cb;
        uint32_t period_usec;
        uint64_t next_usec;        
    } *callbacks;
    uint8_t thread_priority;
    pthread_t thread_ctx;
    bool thread_started;
    AP_HAL::Device *hal_device;
};

#endif // __CYGWIN__