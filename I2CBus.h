#ifndef _I2CBus_h
#define _I2CBus_h

#include <stdint.h>

class I2CBus
{
public:
    I2CBus(const char * deviceName);
    I2CBus(const I2CBus &);
    ~I2CBus();

    void addressSet(uint8_t address);
    void writeByte(uint8_t command, uint8_t data);
    uint8_t readByte(uint8_t command);
    void readBlock(uint8_t command, uint8_t size, uint8_t * data);

private:
    void open_bus();
    int fd;
    const char * deviceName;
};

#endif
