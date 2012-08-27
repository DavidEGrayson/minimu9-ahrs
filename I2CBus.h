#ifndef _I2CBus_h
#define _I2CBus_h

#include <stdint.h>

class I2CBus
{
public:
    I2CBus(const char * devName);
    ~I2CBus();

    void writeByte(int devFd, uint8_t command, uint8_t data);
    uint8_t readByte(int devFd, uint8_t command);
    void readBlock(int devFd, uint8_t command, uint8_t size, uint8_t * data);
    int  registerI2CDevice(uint8_t devAddress);
    void  deregisterI2CDevice(int devFd);

private:
    const char * deviceName;
};

#endif
