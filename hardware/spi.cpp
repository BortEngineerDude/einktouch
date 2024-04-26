#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdexcept>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "spi.h"

#define exception(X)                                                           \
  {                                                                            \
    std::stringstream e;                                                       \
    e << X << ": " << strerror(errno);                                         \
    throw std::runtime_error(e.str());                                         \
  }

namespace spi {

device::device(const std::filesystem::__cxx11::path &device)
    : m_file_path(device) {}

device::~device() { close(); }

void device::write(const char *data, uint byte_count) {
  spi_ioc_transfer tr = {};
  tr.tx_buf = reinterpret_cast<unsigned long>(data);
  tr.len = byte_count;
  tr.delay_usecs = m_delay;

  if (ioctl(m_file_descriptor, SPI_IOC_MESSAGE(1), &tr) < 0)
    exception("SPI write failed");
}

void spi::device::write(const vect &payload) {
  spi_ioc_transfer tr = {};
  tr.tx_buf = reinterpret_cast<unsigned long>(payload.data());
  tr.len = payload.size();
  tr.delay_usecs = m_delay;

  if (ioctl(m_file_descriptor, SPI_IOC_MESSAGE(1), &tr) < 0)
    exception("SPI write failed");
}

void device::write(byte payload) {
  spi_ioc_transfer tr = {};
  tr.tx_buf = reinterpret_cast<unsigned long>(&payload);
  tr.len = 1;
  tr.delay_usecs = m_delay;

  if (ioctl(m_file_descriptor, SPI_IOC_MESSAGE(1), &tr) < 0)
    exception("SPI write failed");
}

vect device::read(uint bytes) {
  vect result(bytes, 0);

  spi_ioc_transfer tr = {};
  tr.rx_buf = reinterpret_cast<unsigned long>(result.data());
  tr.len = bytes;
  tr.delay_usecs = m_delay;
  tr.speed_hz = m_frequency;

  if (ioctl(m_file_descriptor, SPI_IOC_MESSAGE(1), &tr) < 0)
    exception("SPI read failed");

  return result;
}

void device::exchange(vect &payload) {
  spi_ioc_transfer tr[1];
  tr[0].tx_buf = reinterpret_cast<unsigned long>(payload.data());
  tr[0].rx_buf = reinterpret_cast<unsigned long>(payload.data());
  tr[0].len = payload.size();
  tr[0].delay_usecs = m_delay;
  tr[0].speed_hz = m_frequency;

  if (ioctl(m_file_descriptor, SPI_IOC_MESSAGE(1), &(tr)) < 0)
    exception("SPI exchange failed");
}

void device::set_bits_per_word(uint8_t bits) {
  if (ioctl(m_file_descriptor, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0)
    exception("Failed to set SPI bits per word");
}

void device::set_data_delay(uint16_t us) { m_delay = us; }

void device::set_bus_frequency(uint32_t hz) {
  m_frequency = hz;

  if (ioctl(m_file_descriptor, SPI_IOC_WR_MAX_SPEED_HZ, &m_frequency) < 0)
    exception("Failed to set write SPI bus frequency");

  if (ioctl(m_file_descriptor, SPI_IOC_RD_MAX_SPEED_HZ, &m_frequency) < 0)
    exception("Failed to set read SPI bus frequency");
}

void device::set_mode(chip_select cs, bit_order o, bus_mode bm, spi_mode sm) {
  byte mode = 0;

  // Yeah, I know that basic int values 0 to 3 translates exactly to spi modes,
  // but just in case it does not for any reason...
  switch (sm) {
  case spi_mode::_0:
    mode &= ~SPI_CPOL;
    mode &= ~SPI_CPHA;
    break;

  case spi_mode::_1:
    mode &= ~SPI_CPOL;
    mode |= SPI_CPHA;
    break;

  case spi_mode::_2:
    mode |= SPI_CPOL;
    mode &= ~SPI_CPHA;
    break;

  case spi_mode::_3:
    mode |= SPI_CPOL;
    mode |= SPI_CPHA;
    break;
  }

  switch (cs) {
  case chip_select::active_high:
    mode |= SPI_CS_HIGH;
    mode &= ~SPI_NO_CS;
    break;

  case chip_select::active_low:
    mode &= ~SPI_CS_HIGH;
    mode &= ~SPI_NO_CS;
    break;

  case chip_select::manual:
    mode &= ~SPI_CS_HIGH;
    mode |= SPI_NO_CS;
    break;
  }

  if (o == bit_order::lsb_first)
    mode |= SPI_LSB_FIRST;
  else
    mode &= ~SPI_LSB_FIRST;

  if (bm == bus_mode::three_wire)
    mode |= SPI_3WIRE;
  else
    mode &= ~SPI_3WIRE;

  if (ioctl(m_file_descriptor, SPI_IOC_WR_MODE, &mode) < 0)
    exception("Failed to set SPI mode");
}

void device::open() {
  m_file_descriptor = ::open(m_file_path.c_str(), O_RDWR);
  if (m_file_descriptor < 0) {
    std::stringstream error;
    error << "Failed to open SPI device " << m_file_path << strerror(errno);
    throw std::runtime_error(error.str());
  }
}

bool device::is_open() { return m_file_descriptor > 0; }

void device::close() {
  if (m_file_descriptor < 0)
    return;

  ::close(m_file_descriptor);
  m_file_descriptor = -1;
}

} // namespace spi
