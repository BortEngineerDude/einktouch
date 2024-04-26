// By gh/BortEngineerDude
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "i2c.h"

#include <iostream>

namespace i2c {

void controller::write(const byte devaddr, const vect &reg,
                       const vect &payload) {
  i2c_msg messages[1];
  i2c_rdwr_ioctl_data write[1];

  vect output = reg;
  std::copy(payload.begin(), payload.end(), std::back_inserter(output));

  messages[0].addr = static_cast<uint8_t>(devaddr);
  messages[0].flags = 0;
  messages[0].len = output.size();
  messages[0].buf = reinterpret_cast<uint8_t *>(output.data());

  write[0].msgs = messages;
  write[0].nmsgs = 1;

  if (ioctl(m_file_descriptor, I2C_RDWR, &write) < 0) {
    std::stringstream error;
    error << "i2c::controller write failed with error: " << strerror(errno);
    throw std::runtime_error(error.str());
  }
}

vect controller::read(const byte devaddr, const vect &reg, const uint bytes) {
  i2c_msg messages[2];
  i2c_rdwr_ioctl_data exchange[1];
  vect result(bytes);

  messages[0].addr = static_cast<uint8_t>(devaddr);
  messages[0].flags = 0;
  messages[0].len = reg.size();
  messages[0].buf = reinterpret_cast<uint8_t *>(const_cast<byte *>(reg.data()));

  messages[1].addr = static_cast<uint8_t>(devaddr);
  messages[1].flags = I2C_M_RD;
  messages[1].len = bytes;
  messages[1].buf = reinterpret_cast<uint8_t *>(result.data());

  exchange[0].msgs = messages;
  exchange[0].nmsgs = 2;

  if (ioctl(m_file_descriptor, I2C_RDWR, &exchange) < 0) {
    std::stringstream error;
    error << "i2c::controller read failed with error: " << strerror(errno);
    throw std::runtime_error(error.str());
  }

  return result;
}

controller::controller(const fs::path &adapter) : m_file_path(adapter) {}

controller::~controller() { close(); }

void controller::open() {
  m_file_descriptor = ::open(m_file_path.c_str(), O_RDWR);
  if (m_file_descriptor < 0) {
    std::stringstream error;
    error << "i2c::controller failed to open " << m_file_path << ": "
          << strerror(errno);
    throw std::runtime_error(error.str());
  }
}

bool controller::is_open() { return m_file_descriptor > 0; }

void controller::close() {
  if (m_file_descriptor < 0)
    return;

  ::close(m_file_descriptor);

  m_file_descriptor = -1;
}

peripheral::peripheral(controller::ptr controller, const byte address)
    : m_controller(controller), m_address(address) {}

void peripheral::write(const vect &reg, const vect &payload) {
  m_controller->write(m_address, reg, payload);
}

vect peripheral::read(const vect &reg, const uint bytes) {
  return m_controller->read(m_address, reg, bytes);
}

} // namespace i2c
