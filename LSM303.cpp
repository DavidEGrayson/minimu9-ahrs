#include "LSM303.h"
#include <linux/i2c-dev.h>
#include <stdio.h>

#define MAG_ADDRESS            (0x3C >> 1)
#define ACC_ADDRESS_SA0_A_LOW  (0x30 >> 1)
#define ACC_ADDRESS_SA0_A_HIGH (0x32 >> 1)

// TODO: real error handling.  Maybe we should use exceptions.
// TODO: make i2c device class that LSM303 inherits from

LSM303::LSM303(int fd) : fd(fd)
{
    // nothing to do here
}

void LSM303::setAddr(uint8_t addr)
{
    // Specify the address of the slave device.
    if (ioctl(fd, I2C_SLAVE, MAG_ADDRESS) == -1)
    {
        perror("Failed to set I2C_SLAVE address.");
    }
}

void LSM303::addressMag(void)
{
    setAddr(MAG_ADDRESS);
}

void LSM303::addressAcc(void)
{
    setAddr(ACC_ADDRESS_SA0_A_LOW);
}

uint8_t LSM303::readMagReg(int8_t reg)
{
    addressMag();

    // on error, the function below returns -1 which gets converted to 0xFF
    return i2c_smbus_read_byte_data(fd, reg);
}

void LSM303::readAcc(void)
{
    addressAcc();

    uint8_t block[6];

    int result = i2c_smbus_read_i2c_block_data(fd, 0x80 | LSM303_OUT_X_L_A, sizeof(block), block);
    if (result != sizeof(block))
    {
        perror("Failed to read 6-byte accelometer block from LSM303.");
        // TODO: better error handling here and in general
    }

    // DLM, DLHC: register address order is X,Z,Y with high bytes first
    a(0) = block[0] + (block[1] << 8);
    a(1) = block[2] + (block[3] << 8);
    a(2) = block[4] + (block[5] << 8);
}

void LSM303::readMag(void)
{
    addressMag();

    uint8_t block[6];

    int result = i2c_smbus_read_i2c_block_data(fd, 0x80 | LSM303_OUT_X_H_M, sizeof(block), block);
    if (result != sizeof(block))
    {
        perror("Failed to read 6-byte magnetometer block from LSM303.");
        // TODO: better error handling here and in general
    }

    // DLM, DLHC: register address order is X,Z,Y with high bytes first
    m(0) = block[1] + (block[0] << 8);
    m(1) = block[5] + (block[4] << 8);
    m(2) = block[3] + (block[2] << 8);
}

// Reads all 6 channels of the LSM303 and stores them in the object variables
void LSM303::read(void)
{
    readAcc();
    readMag();
}
