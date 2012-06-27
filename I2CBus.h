#ifndef _I2CBus_h
#define _I2CBus_h

#include <stdint.h>

class I2CBus
{
public:
    I2CBus(int fd);

    void setAddress(uint8_t address);
    void writeByte(uint8_t command, uint8_t data);
    uint8_t readByte(uint8_t command);
    void readBlock(uint8_t command, uint8_t size, uint8_t * data);

private:
    int fd;
    int currentAddress;
};

#endif
