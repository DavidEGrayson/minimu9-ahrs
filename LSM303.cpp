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

uint8_t LSM303::readMagReg(int8_t reg)
{
    // Specify the address of the slave device.
    if (ioctl(fd, I2C_SLAVE, MAG_ADDRESS) == -1)
    {
        perror("Failed to set I2C_SLAVE address.");
    }

    // on error, the function below returns -1 which gets converted to 0xFF
    return i2c_smbus_read_byte_data(fd, reg);
}
