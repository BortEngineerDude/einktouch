#pragma once

#include <filesystem>
#include <stdint.h>

#include "bitmap.h"
#include <byte_util.h>

namespace ui {

class bmp_image {
public:
  using path = std::filesystem::path;
  using vect = bytes::vect;

  struct bmp_file_header {
    // Hardcoded size is here to avoid possible alignment issues. No, I will not
    // use pragma(packed)+memcpy, as unaligned memory access might
    // catastrophically fail on some CPUs.
    static const int header_size = 14;
    uint16_t file_type = 0;
    uint32_t file_size = 0;
    uint16_t reserved_1 = 0;
    uint16_t reserved_2 = 0;
    uint32_t data_offset = 0;
  };

  struct bmp_info_header {
    uint32_t info_header_size = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    uint16_t planes = 0;
    uint16_t bits_per_pixel = 0;
    uint32_t compression_type = 0;
    uint32_t image_data_size = 0;
    uint32_t x_pixels_per_meter = 0;
    uint32_t y_pixels_per_meter = 0;
    uint32_t color_table_entries = 0;
    uint32_t important_colors = 0;
    uint bytes_per_row() const;
    uint padding_bytes() const;
  };

  struct color_table_entry {
    static const int size = 4;
    uint8_t blue = 0;
    uint8_t green = 0;
    uint8_t red = 0;
    uint8_t reserved = 0;
  };

  static bmp_image load(const path &p);
  void save(const path &p) const;

  bmp_image to_monochrome(bytes::byte threshold_brightness = 130) const;
  bitmap to_bitmap(bytes::byte threshold_brightness = 130) const;

  vect raw_data() const;

private:
  bmp_file_header m_file_header;
  bmp_info_header m_info_header;
  std::vector<color_table_entry> m_color_table;
  vect m_data;
};

} // namespace ui
