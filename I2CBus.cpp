#include "I2CBus.h"
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <cerrno>
#include <unistd.h>
//#include <system_error>

// TODO: throw some nicer type of exception that results in a nice error message

I2CBus::I2CBus(const char * devName)
{
    fd = open(devName, O_RDWR);
    if (fd == -1)
    {
        perror(devName); // TODO: remove this if a nicer exeption is thrown below
        throw errno;
    }
}

// Copy constructor
I2CBus::I2CBus(const I2CBus & source)
{
    fd = dup(source.fd);
    fprintf(stderr, "dup fd %d -> fd %d\n", source.fd, fd);
    if (fd == -1)
    {
        throw errno;
    }
}

I2CBus::~I2CBus()
{
    fprintf(stderr, "close fd %d\n", fd);
    close(fd);
}

void I2CBus::addressSet(uint8_t address)
{
    fprintf(stderr, "set addr for %d to 0x%02X\n", fd, address);
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

