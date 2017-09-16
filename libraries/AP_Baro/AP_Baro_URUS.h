/*
  dummy backend for URUS (and SITL). This doesn't actually need to do
  any work, as setURUS() is in the frontend
 */
#pragma once

#include "AP_Baro_Backend.h"

#if CONFIG_HAL_BOARD == HAL_BOARD_URUS

class AP_Baro_URUS : public AP_Baro_Backend
{
public:
    AP_Baro_URUS(AP_Baro &baro);
    void update(void);

private:
    uint8_t _instance;
};

#endif // CONFIG_HAL_BOARD == HAL_BOARD_URUS
