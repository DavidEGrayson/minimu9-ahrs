#include "LSM303.h"
#include "i2c-dev.h"
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

uint8_t LSM303::readMagReg(int8_t reg)
{
    addressMag();

    // on error, the function below returns -1 which gets converted to 0xFF
    return i2c_smbus_read_byte_data(fd, reg);
}

void LSM303::readAcc(void)
{
    // TODO: implement this
}

void LSM303::readMag(void)
{
    addressMag();

    uint8_t block[64];

    int result = i2c_smbus_read_block_data(fd, LSM303_OUT_X_H_M, block);
    printf("block: %d %02X%02X%02X%02X%02X%02X\n", result, block[0], block[1], block[2], block[3], block[4], block[5]);
}

// Reads all 6 channels of the LSM303 and stores them in the object variables
void LSM303::read(void)
{
    readAcc();
    readMag();
}
