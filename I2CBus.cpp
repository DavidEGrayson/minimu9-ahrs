#include "I2CBus.h"
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <cerrno>
//#include <system_error>

I2CBus::I2CBus(int fd) : fd(fd), currentAddress(-1)
{
    // nothing to do here
}

void I2CBus::setAddress(uint8_t address)
{
    if (address == currentAddress){ return; }

    int result = ioctl(fd, I2C_SLAVE, address);
    if (result == -1)
    {
        throw errno;
        //throw "Error setting slave address.";
    }

    address = currentAddress;
}

void I2CBus::writeByte(uint8_t command, uint8_t data)
{
    int result = i2c_smbus_write_byte_data(fd, command, data);
    if (result == -1)
    {
        throw errno;
        //throw "Error writing i2c byte.";
    }
}

uint8_t I2CBus::readByte(uint8_t command)
{
    int result = i2c_smbus_read_byte_data(fd, command);
    if (result == -1)
    {
        throw errno;
        //throw "Error reading i2c byte.";
    }
    return result;
}


void I2CBus::readBlock(uint8_t command, uint8_t size, uint8_t * data)
{
    int result = i2c_smbus_read_i2c_block_data(fd, command, size, data);
    if (result != size)
    {
        throw errno;
        //throw "Error reading i2c block.";
    }
}
