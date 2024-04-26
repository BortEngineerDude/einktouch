#pragma once

#include <byte_util.h>
#include <filesystem>
#include <linux/spi/spi.h>

namespace spi {
namespace fs = std::filesystem;
using namespace bytes;

enum class chip_select {
  active_low = 0, // Controlled by SPI, enabled by pulling line low
  active_high,    // Controlled by SPI, enabled by pulling line high
  manual          // SPI has no control over CS, control it by other means
};
enum class bit_order { lsb_first = 0, msb_first };
enum class bus_mode { three_wire = 0, four_wire };
enum class spi_mode {
  _0 = 0, // CPOL = 0, CPHA = 0
  _1,     // CPOL = 0, CPHA = 1
  _2,     // CPOL = 1, CPHA = 0
  _3      // CPOL = 1, CPHA = 1
};

class device {
protected:
  device(const fs::path &device);
  ~device();

  void write(const char *data, uint byte_count);
  void write(const vect &payload);
  void write(byte payload);
  vect read(uint bytes);
  void exchange(vect &payload);

  void set_bits_per_word(uint8_t bits);
  void set_data_delay(uint16_t us);
  void set_bus_frequency(uint32_t hz);

  void set_mode(chip_select cs, bit_order o, bus_mode bm, spi_mode sm);

private:
  fs::path m_file_path;
  int m_file_descriptor = -1;
  uint16_t m_delay = 0;
  uint32_t m_frequency = 500'000;

public:
  void open();
  bool is_open();
  void close();
};

} // namespace spi
