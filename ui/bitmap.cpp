#include "bitmap.h"

#include "bmp_image.h"

#include <math_util.h>

namespace ui {

bitmap::bitmap(const vect &data, const size s)
    : m_data(data), m_geometry(0, 0, s.width(), s.height()) {}

bitmap::bitmap(const size &size) : m_geometry({}, size) {
  auto bytes = div_ceil(size.width(), 8) * size.height();
  m_data.resize(bytes, 0xff);
}

bitmap::bitmap(const bmp_image &img) { *this = img.to_bitmap(); }

geometry::rect bitmap::geometry() const { return m_geometry; }

bitmap::vect &bitmap::raw_data() { return m_data; }

const bitmap::vect &bitmap::raw_data() const { return m_data; }

uint bitmap::bytes_per_row() const {
  return div_ceil(m_geometry.size().width(), 8);
}

bool bitmap::pixel(const point &p) const {
  auto byte_bit = std::div(p.x(), 8);
  auto idx = div_ceil(m_geometry.size().width(), 8) * p.y() + byte_bit.quot;
  if (idx < 0 || idx >= m_data.size())
    throw std::out_of_range("Pixel out of bitmap bounds");

  return m_data[idx] & (0x80 >> byte_bit.rem);
}

bool bitmap::pixel(uint x, uint y) const {
  auto byte_bit = std::div(x, 8);
  auto idx = div_ceil(m_geometry.size().width(), 8) * y + byte_bit.quot;
  if (idx < 0 || idx >= m_data.size())
    throw std::out_of_range("Pixel out of bitmap bounds");

  return m_data[idx] & (0x80 >> byte_bit.rem);
}

void bitmap::draw_pixel(const point &p, bool white) {
  auto byte_bit = std::div(p.x(), 8);
  auto idx = div_ceil(m_geometry.size().width(), 8) * p.y() + byte_bit.quot;
  if (idx < 0 || idx >= m_data.size())
    throw std::out_of_range("Pixel out of bitmap bounds");

  if (white)
    m_data[idx] |= (0x80 >> byte_bit.rem);
  else
    m_data[idx] &= ~(0x80 >> byte_bit.rem);
}

void bitmap::draw_pixel(uint x, uint y, bool white) {
  auto byte_bit = std::div(x, 8);
  auto idx = div_ceil(m_geometry.size().width(), 8) * y + byte_bit.quot;
  if (idx < 0 || idx >= m_data.size())
    throw std::out_of_range("Pixel out of bitmap bounds");

  if (white)
    m_data[idx] |= (0x80 >> byte_bit.rem);
  else
    m_data[idx] &= ~(0x80 >> byte_bit.rem);
}

void bitmap::crop(const rect &area) { *this = cropped(area); }

bitmap bitmap::cropped(const rect &area) const {
  bitmap result;

  result.m_geometry = m_geometry.overlap(area);
  result.m_geometry.normalize();
  auto size = result.m_geometry.size();
  result.m_data.resize(bytes_per_row() * size.height(), 0xff);

  auto [min_x, min_y] = result.m_geometry.top_left().coords();
  auto [max_x, max_y] = result.m_geometry.bottom_right().coords();

  for (int src_y = min_y, dst_y = 0; src_y < max_y; ++src_y, ++dst_y)
    for (int src_x = min_x, dst_x = 0; src_x < max_x; ++src_x, ++dst_x)
      result.draw_pixel(dst_x, dst_y, pixel(src_x, src_y));

  result.m_geometry.set_position(0, 0);

  return result;
}

} // namespace ui
