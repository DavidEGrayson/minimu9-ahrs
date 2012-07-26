#ifndef _IMU_H
#define _IMU_H

#include "vector.h"

class IMU {
public:
    // Scaled readings
    virtual vector readAcc() = 0;  // In body coords, with units = g
    virtual vector readMag() = 0;  // In body coords, scaled to -1..1 range
    virtual vector readGyro() = 0; // In body coords, with units = rad/sec
    void read(){ readAcc(); readMag(); readGyro(); }

    virtual void checkConnection() = 0;
    virtual void measureOffsets() = 0;
    virtual void enableSensors() = 0;
    virtual void loadCalibration() = 0;

    vector accel_offset, gyro_offset;
    int_vector mag_min, mag_max;

    // Raw readings are accessible, mainly for debugging.
    int_vector m, a, g;
};

#endif
