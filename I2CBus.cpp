#include "I2CBus.h"
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <cerrno>
#include <unistd.h>
//#include <system_error>

// TODO: throw some nicer type of exception that results in a nice error message

I2CBus::I2CBus(const char * devName) :  deviceName(devName)
{
    //nothing to do here
}


I2CBus::~I2CBus()
{
    //nothing to do here
}


void I2CBus::writeByte(int devFd, uint8_t command, uint8_t data)
{
    int result = i2c_smbus_write_byte_data(devFd, command, data);
    if (result == -1)
    {
        throw errno;
        //throw "Error writing i2c byte.";
    }
}

uint8_t I2CBus::readByte(int devFd, uint8_t command)
{
    int result = i2c_smbus_read_byte_data(devFd, command);
    if (result == -1)
    {
        throw errno;
        //throw "Error reading i2c byte.";
    }
    return result;
}


void I2CBus::readBlock(int devFd, uint8_t command, uint8_t size, uint8_t * data)
{
    int result = i2c_smbus_read_i2c_block_data(devFd, command, size, data);
    if (result != size)
    {
        throw errno;
        //throw "Error reading i2c block.";
    }
}

int  I2CBus::registerI2CDevice(uint8_t devAddress)
{
    int devFd = open(deviceName, O_RDWR);
    if (devFd == -1)
    {
        perror(deviceName); // TODO: remove this if a nicer exeption is thrown below
        throw errno;
    }
    int result = ioctl(devFd, I2C_SLAVE, devAddress);
    if (result == -1)
    {
        throw errno;
        //throw "Error setting slave address.";
        return -1;    }

    return dup(devFd);
}

void I2CBus::deregisterI2CDevice(int devFd)
{
    int result = close(devFd);
    if (result == -1)
    {
        throw errno;
        //throw "Error closing file descriptor.";
    }
}

