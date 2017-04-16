#include "i2c_bus.h"
#include <cerrno>
#include <cstring>
#include <system_error>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

static inline std::system_error posix_error(const std::string & what)
{
    return std::system_error(errno, std::system_category(), what);
}

i2c_bus::i2c_bus() : fd(-1)
{
}

i2c_bus::i2c_bus(const std::string & name) : fd(-1)
{
  open(name);
}

i2c_bus::i2c_bus(const i2c_bus & other) : fd(-1)
{
  *this = other;
}

i2c_bus & i2c_bus::operator=(const i2c_bus & other)
{
  if (other.fd == -1)
  {
    close();
  }
  else
  {
    open_from_fd(other.fd);
  }
  return *this;
}

i2c_bus::~i2c_bus()
{
  close();
}

void i2c_bus::open(const std::string & name)
{
  close();
  fd = ::open(name.c_str(), O_RDWR);
  if (fd == -1)
  {
    throw posix_error(std::string("Failed to open I2C device ") + name);
  }
}

void i2c_bus::open_from_fd(int other_fd)
{
  close();
  fd = dup(other_fd);
  if (fd == -1)
  {
    throw posix_error("Failed to dup I2C device");
  }
}

void i2c_bus::close()
{
  if (fd != -1)
  {
    ::close(fd);
    fd = -1;
  }
}

void i2c_bus::write_byte_and_read(uint8_t address, uint8_t command,
  uint8_t * data, size_t size)
{
  i2c_msg messages[2] = {
    { address, 0, 1, (typeof(i2c_msg().buf)) &command },
    { address, I2C_M_RD, (typeof(i2c_msg().len)) size, (typeof(i2c_msg().buf)) data },
  };
  i2c_rdwr_ioctl_data ioctl_data = { messages, 2 };

  int result = ioctl(fd, I2C_RDWR, &ioctl_data);

  if (result != 2)
  {
    throw posix_error("Failed to read from I2C");
  }
}

void i2c_bus::write(uint8_t address, const uint8_t * data, size_t size)
{
  i2c_msg messages[1] = {
    { address, 0, (typeof(i2c_msg().len)) size, (typeof(i2c_msg().buf)) data }
  };
  i2c_rdwr_ioctl_data ioctl_data = { messages, 1 };

  int result = ioctl(fd, I2C_RDWR, &ioctl_data);

  if (result != 1)
  {
    throw posix_error("Failed to write to I2C");
  }
}

int i2c_bus::try_write_byte_and_read(uint8_t address, uint8_t byte,
  uint8_t * data, size_t size)
{
  i2c_msg messages[2] = {
    { address, 0, 1, (typeof(i2c_msg().buf))&byte },
    { address, I2C_M_RD, (typeof(i2c_msg().len))size, (typeof(i2c_msg().buf))data },
  };
  i2c_rdwr_ioctl_data ioctl_data = { messages, 2 };

  int result = ioctl(fd, I2C_RDWR, &ioctl_data);

  if (result != 2)
  {
    return -1;
  }

  return 0;
}
