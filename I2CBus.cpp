#include "I2CBus.h"
#include <linux/i2c-dev.h>
#include <stdio.h>

I2CBus::I2CBus(int fd) : fd(fd), currentAddress(-1)
{
    // nothing to do here
}

void I2CBus::setAddress(uint8_t address)
{
    if (address == currentAddress){ return; }

    if (ioctl(fd, I2C_SLAVE, address) == -1)
    {
        throw "Error setting slave address.";
    }

    address = currentAddress;
}

void I2CBus::writeByte(uint8_t command, uint8_t data)
{
    int result = i2c_smbus_write_byte_data(fd, command, data);
    if (result == -1)
    {
        throw "Error writing i2c byte.";
    }
}

uint8_t I2CBus::readByte(uint8_t command)
{
    int result = i2c_smbus_read_byte_data(fd, command);
    if (result == -1)
    {
        throw "Error reading i2c byte.";
    }
    return result;
}


void I2CBus::readBlock(uint8_t command, uint8_t size, uint8_t * data)
{

}
