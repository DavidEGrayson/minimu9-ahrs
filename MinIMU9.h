#ifndef _MINIMU9_H
#define _MINIMU9_H

#include "IMU.h"
#include "LSM303.h"
#include "L3G.h"

class MinIMU9 : public IMU {
public:
    MinIMU9(const char * i2cDeviceName);

    LSM303 compass;
    L3G gyro;

    virtual vector readAcc();
    virtual vector readMag();
    virtual vector readGyro();

    virtual void enable();
    virtual void loadCalibration();
    virtual void measureOffsets();
};

#endif
