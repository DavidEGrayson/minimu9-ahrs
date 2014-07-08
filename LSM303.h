#ifndef LSM303_h
#define LSM303_h

#include <stdint.h>
#include "I2CBus.h"

// register addresses

#define LSM303_CRA_REG_M         0x00 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_CRB_REG_M         0x01 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_MR_REG_M          0x02 // LSM303DLH, LSM303DLM, LSM303DLHC

#define LSM303_TEMP_OUT_L        0x05 // LSM303D
#define LSM303_TEMP_OUT_H        0x06 // LSM303D
#define LSM303_STATUS_M          0x07 // LSM303D

#define LSM303_SR_REG_M          0x09 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_IRA_REG_M         0x0A // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_IRB_REG_M         0x0B // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_IRC_REG_M         0x0C // LSM303DLH, LSM303DLM, LSM303DLHC

#define LSM303_WHO_AM_I_M        0x0F // LSM303DLM
#define LSM303_WHO_AM_I          0x0F // LSM303D

#define LSM303_INT_CTRL_M        0x12 // LSM303D
#define LSM303_INT_SRC_M         0x13 // LSM303D
#define LSM303_INT_THS_L_M       0x14 // LSM303D
#define LSM303_INT_THS_H_M       0x15 // LSM303D
#define LSM303_OFFSET_X_L_M      0x16 // LSM303D
#define LSM303_OFFSET_X_H_M      0x17 // LSM303D
#define LSM303_OFFSET_Y_L_M      0x18 // LSM303D
#define LSM303_OFFSET_Y_H_M      0x19 // LSM303D
#define LSM303_OFFSET_Z_L_M      0x1A // LSM303D
#define LSM303_OFFSET_Z_H_M      0x1B // LSM303D

#define LSM303_REFERENCE_X       0x1C // LSM303D
#define LSM303_REFERENCE_Y       0x1D // LSM303D
#define LSM303_REFERENCE_Z       0x1E // LSM303D

#define LSM303_CTRL0             0x1F // LSM303D
#define LSM303_CTRL_REG1_A       0x20 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_CTRL1             0x20 // LSM303D
#define LSM303_CTRL_REG2_A       0x21 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_CTRL2             0x21 // LSM303D
#define LSM303_CTRL_REG3_A       0x22 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_CTRL3             0x22 // LSM303D
#define LSM303_CTRL_REG4_A       0x23 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_CTRL4             0x23 // LSM303D
#define LSM303_CTRL_REG5_A       0x24 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_CTRL5             0x24 // LSM303D
#define LSM303_CTRL_REG6_A       0x25 // LSM303DLHC
#define LSM303_CTRL6             0x25 // LSM303D
#define LSM303_HP_FILTER_RESET_A 0x25 // LSM303DLH, LSM303DLM
#define LSM303_REFERENCE_A       0x26 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_CTRL7             0x26 // LSM303D
#define LSM303_STATUS_REG_A      0x27 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_STATUS_A          0x27 // LSM303D

#define LSM303_OUT_X_L_A         0x28 // LSM303DLH, LSM303DLM, LSM303DLHC, LSM303D
#define LSM303_OUT_X_H_A         0x29 // LSM303DLH, LSM303DLM, LSM303DLHC, LSM303D
#define LSM303_OUT_Y_L_A         0x2A // LSM303DLH, LSM303DLM, LSM303DLHC, LSM303D
#define LSM303_OUT_Y_H_A         0x2B // LSM303DLH, LSM303DLM, LSM303DLHC, LSM303D
#define LSM303_OUT_Z_L_A         0x2C // LSM303DLH, LSM303DLM, LSM303DLHC, LSM303D
#define LSM303_OUT_Z_H_A         0x2D // LSM303DLH, LSM303DLM, LSM303DLHC, LSM303D

#define LSM303_FIFO_CTRL_REG_A   0x2E // LSM303DLHC
#define LSM303_FIFO_SRC_REG_A    0x2F // LSM303DLHC

#define LSM303_INT1_CFG_A        0x30 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_IG_CFG1           0x30 // LSM303D
#define LSM303_INT1_SRC_A        0x31 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_IG_SRC1           0x31 // LSM303D
#define LSM303_INT1_THS_A        0x32 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_IG_THS1           0x32 // LSM303D
#define LSM303_INT1_DURATION_A   0x33 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_IG_DUR1           0x33 // LSM303D
#define LSM303_INT2_CFG_A        0x34 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_IG_CFG2           0x34 // LSM303D
#define LSM303_INT2_SRC_A        0x35 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_IG_SRC2           0x35 // LSM303D
#define LSM303_INT2_THS_A        0x36 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_IG_THS2           0x36 // LSM303D
#define LSM303_INT2_DURATION_A   0x37 // LSM303DLH, LSM303DLM, LSM303DLHC
#define LSM303_IG_DUR2           0x37 // LSM303D

#define LSM303_CLICK_CFG_A       0x38 // LSM303DLHC
#define LSM303_CLICK_CFG         0x38 // LSM303D
#define LSM303_CLICK_SRC_A       0x39 // LSM303DLHC
#define LSM303_CLICK_SRC         0x39 // LSM303D
#define LSM303_CLICK_THS_A       0x3A // LSM303DLHC
#define LSM303_CLICK_THS         0x3A // LSM303D
#define LSM303_TIME_LIMIT_A      0x3B // LSM303DLHC
#define LSM303_TIME_LIMIT        0x3B // LSM303D
#define LSM303_TIME_LATENCY_A    0x3C // LSM303DLHC
#define LSM303_TIME_LATENCY      0x3C // LSM303D
#define LSM303_TIME_WINDOW_A     0x3D // LSM303DLHC
#define LSM303_TIME_WINDOW       0x3D // LSM303D
#define LSM303_ACT_THS           0x3E // LSM303D
#define LSM303_ACT_DUR           0x3F // LSM303D

#define LSM303_TEMP_OUT_H_M      0x31 // LSM303DLHC
#define LSM303_TEMP_OUT_L_M      0x32 // LSM303DLHC

// Dummy addresses for registers that have inconsistent addresses.
#define LSM303_OUT_X_H_M         -1
#define LSM303_OUT_X_L_M         -2
#define LSM303_OUT_Y_H_M         -3
#define LSM303_OUT_Y_L_M         -4
#define LSM303_OUT_Z_H_M         -5
#define LSM303_OUT_Z_L_M         -6

// Specific addresses for the dummy addresses above:
#define LSM303DLH_OUT_X_H_M      0x03
#define LSM303DLH_OUT_X_L_M      0x04
#define LSM303DLH_OUT_Y_H_M      0x05
#define LSM303DLH_OUT_Y_L_M      0x06
#define LSM303DLH_OUT_Z_H_M      0x07
#define LSM303DLH_OUT_Z_L_M      0x08

#define LSM303DLM_OUT_X_H_M      0x03
#define LSM303DLM_OUT_X_L_M      0x04
#define LSM303DLM_OUT_Z_H_M      0x05
#define LSM303DLM_OUT_Z_L_M      0x06
#define LSM303DLM_OUT_Y_H_M      0x07
#define LSM303DLM_OUT_Y_L_M      0x08

#define LSM303DLHC_OUT_X_H_M     0x03
#define LSM303DLHC_OUT_X_L_M     0x04
#define LSM303DLHC_OUT_Z_H_M     0x05
#define LSM303DLHC_OUT_Z_L_M     0x06
#define LSM303DLHC_OUT_Y_H_M     0x07
#define LSM303DLHC_OUT_Y_L_M     0x08

#define LSM303D_OUT_X_L_M        0x08
#define LSM303D_OUT_X_H_M        0x09
#define LSM303D_OUT_Y_L_M        0x0A
#define LSM303D_OUT_Y_H_M        0x0B
#define LSM303D_OUT_Z_L_M        0x0C
#define LSM303D_OUT_Z_H_M        0x0D

class LSM303
{
 public:
    int a[3];  // accelerometer readings
    int m[3];  // magnetometer readings

    LSM303(const char * i2cDeviceName);

    void enable(void);

    void writeAccReg(uint8_t reg, uint8_t value);
    uint8_t readAccReg(uint8_t reg);
    void writeMagReg(uint8_t reg, uint8_t value);
    uint8_t readMagReg(uint8_t reg);

    void readAcc(void);
    void readMag(void);
    void read(void);

private:
    I2CBus i2c_mag, i2c_acc;
    enum class Device {
        LSM303DLH,
        LSM303DLM,
        LSM303DLHC,
        LSM303D,
    } device;
};

#endif
