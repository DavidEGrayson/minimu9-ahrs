#ifndef _MINIMU9_H
#define _MINIMU9_H

#include "IMU.h"
#include "I2CBus.h"
#include "LSM303.h"
#include "L3G.h"

class MinIMU9 : public IMU {
public:
    MinIMU9(I2CBus& i2c);

    LSM303 compass;
    L3G gyro;

    virtual vector readAcc();
    virtual vector readMag();
    virtual vector readGyro();

    virtual void checkConnection();
    virtual void enableSensors();
    virtual void loadCalibration();
    virtual void measureOffsets();
};

#endif
