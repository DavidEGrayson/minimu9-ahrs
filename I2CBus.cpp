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
}

void I2CBus::readBlock(uint8_t command, uint8_t size, uint8_t * data)
{

}
