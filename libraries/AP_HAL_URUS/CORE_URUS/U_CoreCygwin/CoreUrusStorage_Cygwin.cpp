#include <AP_HAL_URUS/AP_HAL_URUS.h>
#if CONFIG_HAL_BOARD == HAL_BOARD_URUS

#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "CoreUrusStorage_Cygwin.h"

void CLCoreUrusEEStorage_Cygwin::_eeprom_open(void)
{
    if (_eeprom_fd == -1) {
        _eeprom_fd = open("eeprom.bin", O_RDWR|O_CREAT|O_CLOEXEC, 0777);
        assert(ftruncate(_eeprom_fd, HAL_STORAGE_SIZE) == 0);
    }
}

void CLCoreUrusEEStorage_Cygwin::read_block(void *dst, uint16_t src, size_t n)
{
    assert(src < HAL_STORAGE_SIZE && src + n <= HAL_STORAGE_SIZE);
    _eeprom_open();
    assert(pread(_eeprom_fd, dst, n, src) == (ssize_t)n);
}

void CLCoreUrusEEStorage_Cygwin::write_block(uint16_t dst, const void *src, size_t n)
{
    assert(dst < HAL_STORAGE_SIZE);
    _eeprom_open();
    assert(pwrite(_eeprom_fd, src, n, dst) == (ssize_t)n);
}

#endif // CONFIG_HAL_BOARD == HAL_BOARD_URUS
