#include "I2CBus.h"
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <cerrno>
#include <unistd.h>
//#include <system_error>

// TODO: throw some nicer type of exception that results in a nice error message

I2CBus::I2CBus(const char * deviceName) : deviceName(deviceName)
{
    open_bus();
}

// Copy constructor
I2CBus::I2CBus(const I2CBus & source) : deviceName(source.deviceName)
{
    open_bus();
}

void I2CBus::open_bus()
{
    fd = open(deviceName, O_RDWR);
    if (fd == -1)
    {
        perror(deviceName); // TODO: remove this if a nicer exeption is thrown below
        throw errno;
    }
}

I2CBus::~I2CBus()
{
    fprintf(stderr, "closing fd=%d\n",fd);
    close(fd);
}

void I2CBus::addressSet(uint8_t address)
{
    int result = ioctl(fd, I2C_SLAVE, address);
    if (result == -1)
    {
        throw errno;
    }
}

void I2CBus::writeByte(uint8_t command, uint8_t data)
{
    int result = i2c_smbus_write_byte_data(fd, command, data);
    if (result == -1)
    {
        throw errno;
    }
}

uint8_t I2CBus::readByte(uint8_t command)
{
    int result = i2c_smbus_read_byte_data(fd, command);
    if (result == -1)
    {
        throw errno;
    }
    return result;
}

void I2CBus::readBlock(uint8_t command, uint8_t size, uint8_t * data)
{
    int result = i2c_smbus_read_i2c_block_data(fd, command, size, data);
    if (result != size)
    {
        throw errno;
    }
}
