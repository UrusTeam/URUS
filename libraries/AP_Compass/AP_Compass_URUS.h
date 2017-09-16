#pragma once

#include "AP_Compass.h"

#define URUS_NUM_COMPASSES 2

class AP_Compass_URUS : public AP_Compass_Backend
{
public:
    AP_Compass_URUS(Compass &compass);
    void read(void);
    bool init(void);

    // detect the sensor
    static AP_Compass_Backend *detect(Compass &compass);

private:
    uint8_t     _compass_instance[URUS_NUM_COMPASSES];
};
