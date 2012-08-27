#ifndef _I2CBus_h
#define _I2CBus_h

#include <stdint.h>

class I2CBus
{
public:
    I2CBus(int fd);
    I2CBus(const char * devName);
    ~I2CBus();

//    void setAddress(uint8_t address);
    void writeByte(int devFd, uint8_t command, uint8_t data);
    uint8_t readByte(int devFd, uint8_t command);
    void readBlock(int devFd, uint8_t command, uint8_t size, uint8_t * data);
    int  registerI2CDevice(uint_8_t devAddress);
    void  deregisterI2CDevice(devFd);

private:
    int fd;
    bool ownFd;
    //    int currentAddress;
};

#endif
