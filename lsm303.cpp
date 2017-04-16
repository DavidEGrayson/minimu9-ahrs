#include "lsm303.h"
#include <stdexcept>

// Relevant Pololu products:
//
// #1250  LSM303DLH                  SA0_A pulled to GND, accessible via.
// #1264  LSM303DLH + L3G4200D (v0)  SA0_A pulled to GND, accessible thru-hole.
// #1265  LSM303DLM + L3G4200D (v1)  SA0_A pulled to GND, accessible thru-hole.
// #1268  LSM303DLHC + L3GD20  (v2)  (chip has no SA0 line)
// #1273  LSM303DLM                  SA0_A pulled to GND, accessible via.
// #2124  LSM303DLHC                 (chip has no SA0 line)
// #2127  LSM303D                    SA0 pulled to VDD, accessible thru-hole.
// #2468  LSM303D + L3GD20H (v3)     SA0 pulled to VDD, accessible thru-hole

void lsm303::handle::open(const comm_config & config)
{
  if (!config.use_sensor)
  {
    throw std::runtime_error("LSM303 configuration is null.");
  }

  this->config = config;
  i2c.open(config.i2c_bus_name);
}

uint8_t lsm303::handle::read_mag_reg(uint8_t reg)
{
  return i2c.write_byte_and_read_byte(config.i2c_address_mag, reg);
}

uint8_t lsm303::handle::read_acc_reg(uint8_t reg)
{
  return i2c.write_byte_and_read_byte(config.i2c_address_acc, reg);
}

void lsm303::handle::write_mag_reg(uint8_t reg, uint8_t value)
{
  i2c.write_two_bytes(config.i2c_address_mag, reg, value);
}

void lsm303::handle::write_acc_reg(uint8_t reg, uint8_t value)
{
  i2c.write_two_bytes(config.i2c_address_acc, reg, value);
}

// Turns on the LSM303's accelerometer and magnetometers and places them in normal
// mode.
void lsm303::handle::enable()
{
  if (config.device == LSM303D)
  {
    //// LSM303D Accelerometer

    // AODR = 0101 (50 Hz ODR)
    // AZEN = AYEN = AXEN = 1 (all axes enabled)
    write_acc_reg(CTRL1, 0b01010111);

    // AFS = 011 (8 g full scale)
    write_acc_reg(CTRL2, 0b00011000);

    //// LSM303D Magnetometer
    // M_RES = 11 (high resolution mode)
    // M_ODR = 001 (6.25 Hz ODR)
    write_mag_reg(CTRL5, 0b01100100);

    // MFS = 01 (4 gauss full scale)
    write_mag_reg(CTRL6, 0b00100000);

    // MLP = 0 (low power mode off)
    // MD = 00 (continuous-conversion mode)
    write_mag_reg(CTRL7, 0b00000000);
  }
  else if (config.device == LSM303DLHC)
  {
    //// LSM303DLHC Accelerometer

    // ODR = 0100 (50 Hz ODR)
    // LPen = 0 (normal mode)
    // Zen = Yen = Xen = 1 (all axes enabled)
    write_acc_reg(CTRL_REG1_A, 0b01000111);

    // FS = 10 (8 g full scale)
    // HR = 1 (high resolution enable)
    write_acc_reg(CTRL_REG4_A, 0b00101000);

    //// LSM303DLHC Magnetometer

    // DO = 011 (7.5 Hz ODR)
    write_mag_reg(CRA_REG_M, 0b00001100);

    // GN = 001 (+/- 1.3 gauss full scale)
    write_mag_reg(CRB_REG_M, 0b00100000);

    // MD = 00 (continuous-conversion mode)
    write_mag_reg(MR_REG_M, 0b00000000);
  }
  else
  {
    //// LSM303DLM or LSM303DLH Accelerometer

    // FS = 11 (8 g full scale)
    write_acc_reg(CTRL_REG4_A, 0b00110000);

    // PM = 001 (normal mode)
    // DR = 00 (50 Hz ODR)
    // Zen = Yen = Xen = 1 (all axes enabled)
    write_acc_reg(CTRL_REG1_A, 0b00100111);

    //// LSM303DLM or LSM303DLH Magnetometer

    // DO = 011 (7.5 Hz ODR)
    write_mag_reg(CRA_REG_M, 0b00001100);

    // GN = 001 (+/- 1.3 gauss full scale)
    write_mag_reg(CRB_REG_M, 0b00100000);

    // MD = 00 (continuous-conversion mode)
    write_mag_reg(MR_REG_M, 0b00000000);
  }
}

void lsm303::handle::read_acc()
{
  uint8_t block[6];
  i2c.write_byte_and_read(config.i2c_address_acc,
    0x80 | OUT_X_L_A, block, sizeof(block));
  a[0] = (int16_t)(block[0] | block[1] << 8);
  a[1] = (int16_t)(block[2] | block[3] << 8);
  a[2] = (int16_t)(block[4] | block[5] << 8);
}

void lsm303::handle::read_mag()
{
  uint8_t block[6];

  if (config.device == LSM303D)
  {
    // LSM303D: XYZ order, little endian
    i2c.write_byte_and_read(config.i2c_address_mag,
      0x80 | D_OUT_X_L_M, block, sizeof(block));
    m[0] = (int16_t)(block[0] | block[1] << 8);
    m[1] = (int16_t)(block[2] | block[3] << 8);
    m[2] = (int16_t)(block[4] | block[5] << 8);
  }
  else if (config.device == LSM303DLH)
  {
    // LSM303DLH: XYZ order, big endian
    i2c.write_byte_and_read(config.i2c_address_mag,
      0x80 | DLH_OUT_X_H_M, block, sizeof(block));
    m[0] = (int16_t)(block[1] | block[0] << 8);
    m[1] = (int16_t)(block[3] | block[2] << 8);
    m[2] = (int16_t)(block[5] | block[4] << 8);
  }
  else
  {
    // LSM303DLM, LSM303DLHC: XZY order, big endian (and same addresses)
    i2c.write_byte_and_read(config.i2c_address_mag,
      0x80 | DLM_OUT_X_H_M, block, sizeof(block));
    m[0] = (int16_t)(block[1] | block[0] << 8);
    m[1] = (int16_t)(block[5] | block[4] << 8);
    m[2] = (int16_t)(block[3] | block[2] << 8);
  }
}

void lsm303::handle::read()
{
  read_acc();
  read_mag();
}
