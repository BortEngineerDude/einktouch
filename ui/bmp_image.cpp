#include <algorithm>
#include <fstream>
#include <math.h>
#include <sstream>

#include "bmp_image.h"

struct rgb_pixel {
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  uint8_t perceived_brightness() const {
    return sqrt(.241 * red * red + .691 * green * green + .068 * blue * blue);
  }
};

template <typename B>
const bytes::buffer<B> &operator>>(const bytes::buffer<B> &buff,
                                   rgb_pixel &pxl) {
  buff >> pxl.red >> pxl.green >> pxl.blue;
  return buff;
}

uint ui::bmp_image::bmp_info_header::bytes_per_row() const {
  return (width * bits_per_pixel + 31) / 32 * 4;
}

uint ui::bmp_image::bmp_info_header::padding_bytes() const {
  return bytes_per_row() - width * bits_per_pixel / 8;
}

namespace ui {

bmp_image bmp_image::load(const path &p) {
  if (!std::filesystem::exists(p)) {
    std::stringstream error;
    error << "File " << p << " does not exist";
    throw std::runtime_error(error.str());
  }

  bmp_image result;
  std::ifstream input;
  input.open(p, std::ios::binary | std::ios::in);

  if (!input.good()) {
    std::stringstream error;
    error << "Failed to open file: " << p;
    throw std::runtime_error(error.str());
  }

  // Read BMP file header and the first byte of DIB (BMP info header)
  vect raw_data(bmp_file_header::header_size +
                sizeof(bmp_info_header::info_header_size));
  input.read(reinterpret_cast<char *>(raw_data.data()), raw_data.size());
  const bytes::buffer parser(raw_data, std::endian::little);

  auto &info = result.m_info_header;
  auto &header = result.m_file_header;

  parser >> header.file_type >> header.file_size >> header.reserved_1 >>
      header.reserved_2 >> header.data_offset >> info.info_header_size;

  if (header.file_type != 0x4d42) // Windows BMP
    throw std::runtime_error("File type not supported.");

  // Read the rest of DIB (BMP info header)
  raw_data.resize(info.info_header_size);
  input.read(reinterpret_cast<char *>(raw_data.data()), raw_data.size());
  parser.seek(0, std::ios_base::beg);

  parser >> info.width >> info.height >> info.planes >> info.bits_per_pixel >>
      info.compression_type >> info.image_data_size >>
      info.x_pixels_per_meter >> info.y_pixels_per_meter >>
      info.color_table_entries >> info.important_colors;

  // Read color table if necessary
  if (info.bits_per_pixel <= 8) {
    auto &color_table = result.m_color_table;

    /*
    Number of color table entries are equal to 2^BPP (bits per pixel) or to the
    number of color_table_entries, which should be greater than 0 and less than
    2^BPP to be valid.
    */
    auto color_table_entries = pow(2, info.bits_per_pixel);
    if (info.color_table_entries &&
        info.color_table_entries < color_table_entries)
      color_table_entries = info.color_table_entries;

    color_table.resize(color_table_entries);
    raw_data.resize(color_table_entry::size * color_table_entries);
    input.read(reinterpret_cast<char *>(raw_data.data()), raw_data.size());
    parser.seek(0, std::ios_base::beg);

    for (int entry = 0; entry < color_table_entries; ++entry)
      parser >> color_table[entry].blue >> color_table[entry].green >>
          color_table[entry].red >> color_table[entry].reserved;
  }

  if (info.image_data_size == 0)
    info.image_data_size = info.bytes_per_row() * info.height;

  input.seekg(header.data_offset, std::ios_base::beg);
  result.m_data.resize(info.image_data_size, 0xff);
  input.read(reinterpret_cast<char *>(result.m_data.data()),
             result.m_data.size());

  return result;
}

bmp_image bmp_image::to_monochrome(bytes::byte threshold_brightness) const {
  if (m_data.empty())
    throw std::runtime_error("No image loaded.");

  if (m_info_header.bits_per_pixel == 1)
    return *this;

  bmp_image result;
  auto &info = result.m_info_header;
  auto &file_header = result.m_file_header;
  auto &data = result.m_data;

  file_header = m_file_header;
  info = m_info_header;
  info.bits_per_pixel = 1;
  info.color_table_entries = 2;
  info.compression_type = 0;

  auto result_bytes_per_row = info.bytes_per_row();
  result.m_color_table.resize(2);
  result.m_color_table[0] = {0, 0, 0, 0};
  result.m_color_table[1] = {255, 255, 255, 0};
  data.resize(info.height * result_bytes_per_row, 0xff);

  if (m_info_header.bits_per_pixel == 24) {
    const bytes::buffer pixel_reader(m_data);
    const auto padding_bytes = m_info_header.padding_bytes();
    rgb_pixel pixel_data = {};
    uint reduced_byte = 0;
    uint reduced_bit = 0;
    bool value = true;

    for (uint row = 0; row < m_info_header.height; ++row) {
      for (uint pixel = 0; pixel < m_info_header.width; ++pixel) {
        auto div = std::div(pixel, 8);
        reduced_byte = div.quot;
        reduced_bit = 7 - div.rem;
        pixel_reader >> pixel_data;
        value = pixel_data.perceived_brightness() >= threshold_brightness;

        result.m_data[row * result_bytes_per_row + reduced_byte] &=
            ~(value << reduced_bit);
      }

      pixel_reader.seek(padding_bytes);
    }

    return result;
  }

  throw std::runtime_error("Conversion of that type isn't supported.");
}

bitmap bmp_image::to_bitmap(bytes::byte threshold_brightness) const {
  auto monochrome = to_monochrome(threshold_brightness);
  auto &info = monochrome.m_info_header;
  auto &data = monochrome.m_data;

  auto bytes_per_row = info.bytes_per_row();

  for (uint row = 0; row < info.height / 2; ++row)
    for (uint byte = 0; byte < bytes_per_row; ++byte)
      std::swap(data[row * bytes_per_row + byte],
                data[(info.height - 1 - row) * bytes_per_row + byte]);

  bitmap result{monochrome.m_data,
                {static_cast<int>(m_info_header.width),
                 static_cast<int>(m_info_header.height)}};

  // Trim away all unnecessary padding
  result.crop(result.geometry());

  return result;
}

bmp_image::vect bmp_image::raw_data() const { return m_data; }

} // namespace ui
