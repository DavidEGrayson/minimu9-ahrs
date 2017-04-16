#pragma once

#include <cstdint>
#include <string>

class i2c_bus
{
public:
  explicit i2c_bus(const std::string & name);
  i2c_bus();

  // Move constructor
  i2c_bus(i2c_bus &&);

  // Copy constructor
  i2c_bus(const i2c_bus &);

  // Move assignment operator
  i2c_bus & operator=(i2c_bus &&);

  // Copy assignment operator
  i2c_bus & operator=(const i2c_bus &);

  ~i2c_bus();

  void open(const std::string & name);
  void open_from_fd(int other_fd);
  void close();

  void write_byte_and_read(uint8_t address, uint8_t byte,
    uint8_t * data, size_t size);

  void write(uint8_t device_address, const uint8_t * data, size_t size);

  int try_write_byte_and_read(uint8_t address, uint8_t byte,
    uint8_t * data, size_t size);

  void write_two_bytes(uint8_t address, uint8_t byte1, uint8_t byte2)
  {
    uint8_t buffer[] = { byte1, byte2 };
    write(address, buffer, 2);
  }

  uint8_t write_byte_and_read_byte(uint8_t address, uint8_t byte1)
  {
    uint8_t byte2;
    write_byte_and_read(address, byte1, &byte2, 1);
    return byte2;
  }

  int try_write_byte_and_read_byte(uint8_t address, uint8_t byte1) noexcept
  {
    uint8_t byte2;
    int result = try_write_byte_and_read(address, byte1, &byte2, 1);
    if (result < 0) { return result; }
    return byte2;
  }

private:
  int fd;
};
