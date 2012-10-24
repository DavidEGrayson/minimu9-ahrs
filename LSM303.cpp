#include "LSM303.h"

/*

Relevant Pololu products:

#1250  LSM303DLH              SA0_A pulled to GND, accessible via.
#1264  LSM303DLH + L3G4200D   SA0_A pulled to GND, accessible thru-hole.
#1265  LSM303DLM + L3G4200D   SA0_A pulled to GND, accessible thru-hole.
#1268  LSM303DLHC + L3GD20
#1273  LSM303DLM              SA0_A pulled to GND, accessible via.
#2124  LSM303DLHC

LSM303DLHC has no SA0_A line

 */

#define MAG_ADDRESS            (0x3C >> 1)
#define ACC_ADDRESS_SA0_A_LOW  (0x30 >> 1)
#define ACC_ADDRESS_SA0_A_HIGH (0x32 >> 1)

LSM303::LSM303(const char * i2cDeviceName) :
  i2c_mag(i2cDeviceName), i2c_acc(i2cDeviceName)
{
    i2c_mag.addressSet(MAG_ADDRESS);

    // Detect the accelerometer address and device.
    i2c_acc.addressSet(ACC_ADDRESS_SA0_A_LOW);
    bool sa0_a_high = i2c_acc.tryReadByte(LSM303_CTRL_REG1_A) == -1;
    if (sa0_a_high)
    {
        i2c_acc.addressSet(ACC_ADDRESS_SA0_A_HIGH);
        // Only the DLHC should be responding on the high address.
        device = Device::LSM303DLHC;
    }
    else
    {
        // Only the DLM has a LSM303_WHO_AM_I_M register.
        device = i2c_mag.tryReadByte(LSM303_WHO_AM_I_M) == 0x3C ? Device::LSM303DLM : Device::LSM303DLH;
    }

    // Make sure to throw an exception if we don't have the right address.
    readAccReg(LSM303_CTRL_REG1_A);

    if (readMagReg(LSM303_WHO_AM_I_M) != 0x3C)
    {
        throw std::runtime_error("Error getting \"Who Am I\" register.\n");
    }
}

uint8_t LSM303::readMagReg(uint8_t reg)
{
    return i2c_mag.readByte(reg);
}

uint8_t LSM303::readAccReg(uint8_t reg)
{
    return i2c_acc.readByte(reg);
}

void LSM303::writeMagReg(uint8_t reg, uint8_t value)
{
    i2c_mag.writeByte(reg, value);
}

void LSM303::writeAccReg(uint8_t reg, uint8_t value)
{
    i2c_acc.writeByte(reg, value);
}

// Turns on the LSM303's accelerometer and magnetometers and places them in normal
// mode.
void LSM303::enable(void)
{
    // Enable accelerometer.
    if (device == Device::LSM303DLHC)
    {    
        writeAccReg(LSM303_CTRL_REG1_A, 0b01000111); // Normal power mode, all axes enabled, 50 Hz
        writeAccReg(LSM303_CTRL_REG4_A, 0x28); // 8 g full scale: FS = 10 on DLHC, high resolution output mode
    }
    else
    {
        writeAccReg(LSM303_CTRL_REG1_A, 0b00100111); // normal power mode, all axes enabled, 50 Hz
        writeAccReg(LSM303_CTRL_REG4_A, 0b00110000); // 8 g full scale: FS = 11 on DLH, DLM
    }

    // Enable magnetometer
    // Continuous conversion mode
    writeMagReg(LSM303_MR_REG_M, 0x00);
}

void LSM303::readAcc(void)
{
    uint8_t block[6];
    i2c_acc.readBlock(0x80 | LSM303_OUT_X_L_A, sizeof(block), block);

    a[0] = (int16_t)(block[0] | block[1] << 8) >> 4;
    a[1] = (int16_t)(block[2] | block[3] << 8) >> 4;
    a[2] = (int16_t)(block[4] | block[5] << 8) >> 4;
}

void LSM303::readMag(void)
{
    uint8_t block[6];
    i2c_mag.readBlock(0x80 | LSM303_OUT_X_H_M, sizeof(block), block);

    // DLM, DLHC: register address order is X,Z,Y with high bytes first
    m[0] = (int16_t)(block[1] | block[0] << 8);
    m[1] = (int16_t)(block[5] | block[4] << 8);
    m[2] = (int16_t)(block[3] | block[2] << 8);

    // TODO: handle DLH properly here (switch two components?)

}

// Reads all 6 channels of the LSM303 and stores them in the object variables
void LSM303::read(void)
{
    readAcc();
    readMag();
}
