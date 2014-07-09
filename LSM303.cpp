#include "LSM303.h"

/*
Relevant Pololu products:

#1250  LSM303DLH                  SA0_A pulled to GND, accessible via.
#1264  LSM303DLH + L3G4200D (v0)  SA0_A pulled to GND, accessible thru-hole.
#1265  LSM303DLM + L3G4200D (v1)  SA0_A pulled to GND, accessible thru-hole.
#1268  LSM303DLHC + L3GD20  (v2)  (chip has no SA0 line)
#1273  LSM303DLM                  SA0_A pulled to GND, accessible via.
#2124  LSM303DLHC                 (chip has no SA0 line)
#2127  LSM303D                    SA0 pulled to VDD, accessible thru-hole.
#2468  LSM303D + L3GD20H (v3)     SA0 pulled to VDD, accessible thru-hole
*/

// LSM303D addresses:
#define D_SA0_HIGH_ADDRESS              0b0011101
#define D_SA0_LOW_ADDRESS               0b0011110

// LSM303DLH, LSM303DLM, LSM303DLHC addresses:
#define NON_D_MAG_ADDRESS               0b0011110
#define NON_D_ACC_SA0_LOW_ADDRESS       0b0011000
#define NON_D_ACC_SA0_HIGH_ADDRESS      0b0011001

#define D_WHO_ID    0x49
#define DLM_WHO_ID  0x3C

LSM303::LSM303(const char * i2cDeviceName) :
  i2c_mag(i2cDeviceName), i2c_acc(i2cDeviceName)
{
    I2CBus i2c(i2cDeviceName);
    bool sa0;

    if (i2c.tryReadByte(D_SA0_HIGH_ADDRESS, LSM303_WHO_AM_I) == D_WHO_ID)
    {
        // Detected LSM303D with SA0 high.
        device = Device::LSM303D;
        sa0 = true;
    }
    else if (i2c.tryReadByte(D_SA0_LOW_ADDRESS, LSM303_WHO_AM_I) == D_WHO_ID)
    {
        // device responds to address 0011110 with D ID; it's a D with SA0 low
        device = Device::LSM303D;
        sa0 = false;
    }
    // Remaining possibilities: LSM303DLHC, LSM303DLM, or LSM303DLH.
    // LSM303DLHC seems to respond to WHO_AM_I request the same way as DLM, even
    // though this register isn't documented in its datasheet.
    // Instead of reading the WHO_AM_I register, guess if it's an LSM303DLHC based on
    // accelerometer address, because Pololu boards with the LSM303DLM or LSM303DLH
    // pull SA0 low.  The LSM303DLHC doesn't have SA0 but uses same accelerometer address
    // as LSM303DLH/LSM303DLM with SA0 high).
    else if (i2c.tryReadByte(NON_D_ACC_SA0_HIGH_ADDRESS, LSM303_CTRL_REG1_A) >= 0)
    {
        // Guess that it's an LSM303DLHC.
        device = Device::LSM303DLHC;
        sa0 = true;
    }
    // Remaining possibilities: LSM303DLM or LSM303DLH.
    // Check accelerometer with SA0 low address to make sure it's responsive.
    else if (i2c.tryReadByte(NON_D_ACC_SA0_LOW_ADDRESS, LSM303_CTRL_REG1_A) >= 0)
    {
        sa0 = false;

        if (i2c.tryReadByte(NON_D_MAG_ADDRESS, LSM303_WHO_AM_I_M) == DLM_WHO_ID)
        {
            // Detected LSM303DLM with SA0 low.
            device = Device::LSM303DLM;
        }
        else
        {
            // Guess that it's an LSM303DLH with SA0 low.
            device = Device::LSM303DLH;
        }
    }
    else
    {
        throw std::runtime_error("Could not detect accelerometer.");
    }

    // Set the I2C addresses.
    if (device == Device::LSM303D)
    {
        uint8_t address = sa0 ? D_SA0_HIGH_ADDRESS : D_SA0_LOW_ADDRESS;
        i2c_acc.addressSet(address);
        i2c_mag.addressSet(address);
    }
    else
    {
        i2c_acc.addressSet(sa0 ? NON_D_ACC_SA0_HIGH_ADDRESS : NON_D_ACC_SA0_LOW_ADDRESS);
        i2c_mag.addressSet(NON_D_MAG_ADDRESS);
    }

    // Make sure we can actually read an accelerometer control register.
    // (This throws an exception if there is a problem.)
    readAccReg(LSM303_CTRL_REG1_A);

    // For non-LSM303 devices, the magnetometer has a different address so
    // we should make sure it is actually there.
    if (device != Device::LSM303D && readMagReg(LSM303_WHO_AM_I_M) != 0x3C)
    {
        throw std::runtime_error("Error getting \"Who Am I\" register for magnetometer.\n");
    }
}

uint8_t LSM303::readMagReg(uint8_t reg)
{
    return i2c_mag.readByte(reg);
}

uint8_t LSM303::readAccReg(uint8_t reg)
{
    return i2c_acc.readByte(reg);
}

void LSM303::writeMagReg(uint8_t reg, uint8_t value)
{
    i2c_mag.writeByte(reg, value);
}

void LSM303::writeAccReg(uint8_t reg, uint8_t value)
{
    i2c_acc.writeByte(reg, value);
}

// Turns on the LSM303's accelerometer and magnetometers and places them in normal
// mode.
void LSM303::enable(void)
{
    if (device == Device::LSM303D)
    {
        //// LSM303D Accelerometer

        // AODR = 0101 (50 Hz ODR)
        // AZEN = AYEN = AXEN = 1 (all axes enabled)
        writeAccReg(LSM303_CTRL1, 0b01010111);

        // AFS = 011 (8 g full scale)
        writeAccReg(LSM303_CTRL2, 0b00011000);

        //// LSM303D Magnetometer
        // M_RES = 11 (high resolution mode)
        // M_ODR = 001 (6.25 Hz ODR)
        writeMagReg(LSM303_CTRL5, 0b01100100);

        // MFS = 01 (4 gauss full scale)
        writeMagReg(LSM303_CTRL6, 0b00100000);

        // MLP = 0 (low power mode off)
        // MD = 00 (continuous-conversion mode)
        writeMagReg(LSM303_CTRL7, 0b00000000);
    }
    else if (device == Device::LSM303DLHC)
    {
        //// LSM303DLHC Accelerometer

        // ODR = 0100 (50 Hz ODR)
        // LPen = 0 (normal mode)
        // Zen = Yen = Xen = 1 (all axes enabled)
        writeAccReg(LSM303_CTRL_REG1_A, 0b01000111);

        // FS = 10 (8 g full scale)
        // HR = 1 (high resolution enable)
        writeAccReg(LSM303_CTRL_REG4_A, 0b00101000);

        //// LSM303DLHC Magnetometer

        // DO = 011 (7.5 Hz ODR)
        writeMagReg(LSM303_CRA_REG_M, 0b00001100);

        // GN = 001 (+/- 1.3 gauss full scale)
        writeMagReg(LSM303_CRB_REG_M, 0b00100000);

        // MD = 00 (continuous-conversion mode)
        writeMagReg(LSM303_MR_REG_M, 0b00000000);
    }
    else
    {
        //// LSM303DLM or LSM303DLH Accelerometer

        // FS = 11 (8 g full scale)
        writeAccReg(LSM303_CTRL_REG4_A, 0b00110000);

        // PM = 001 (normal mode)
        // DR = 00 (50 Hz ODR)
        // Zen = Yen = Xen = 1 (all axes enabled)
        writeAccReg(LSM303_CTRL_REG1_A, 0b00100111);

        //// LSM303DLM or LSM303DLH Magnetometer

        // DO = 011 (7.5 Hz ODR)
        writeMagReg(LSM303_CRA_REG_M, 0b00001100);

        // GN = 001 (+/- 1.3 gauss full scale)
        writeMagReg(LSM303_CRB_REG_M, 0b00100000);

         // MD = 00 (continuous-conversion mode)
        writeMagReg(LSM303_MR_REG_M, 0b00000000);
    }
}

void LSM303::readAcc(void)
{
    uint8_t block[6];
    i2c_acc.readBlock(0x80 | LSM303_OUT_X_L_A, sizeof(block), block);
    a[0] = (int16_t)(block[0] | block[1] << 8);
    a[1] = (int16_t)(block[2] | block[3] << 8);
    a[2] = (int16_t)(block[4] | block[5] << 8);
}

void LSM303::readMag(void)
{
    uint8_t block[6];

    if (device == Device::LSM303D)
    {
        // LSM303D: XYZ order, little endian
        i2c_mag.readBlock(0x80 | LSM303D_OUT_X_L_M, sizeof(block), block);
        m[0] = (int16_t)(block[0] | block[1] << 8);
        m[1] = (int16_t)(block[2] | block[3] << 8);
        m[2] = (int16_t)(block[4] | block[5] << 8);
    }
    else if (device == Device::LSM303DLH)
    {
        // LSM303DLH: XYZ order, big endian
        i2c_mag.readBlock(0x80 | LSM303DLH_OUT_X_H_M, sizeof(block), block);
        m[0] = (int16_t)(block[1] | block[0] << 8);
        m[1] = (int16_t)(block[3] | block[2] << 8);
        m[2] = (int16_t)(block[5] | block[4] << 8);
    }
    else
    {
        // LSM303DLM, LSM303DLHC: XZY order, big endian (and same addresses)
        i2c_mag.readBlock(0x80 | LSM303DLM_OUT_X_H_M, sizeof(block), block);
        m[0] = (int16_t)(block[1] | block[0] << 8);
        m[1] = (int16_t)(block[5] | block[4] << 8);
        m[2] = (int16_t)(block[3] | block[2] << 8);
    }
}

void LSM303::read(void)
{
    readAcc();
    readMag();
}
