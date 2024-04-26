#pragma once

#include <byte_util.h>
#include <rect.h>

namespace ui {

class bmp_image;

/**
 * @brief The bitmap class - a class representing monochrome images (with 1 bit
 * per pixel)
 */
class bitmap {
public:
private:
  using vect = bytes::vect;
  using rect = geometry::rect;
  using size = geometry::size;
  using point = geometry::point;

  vect m_data;
  rect m_geometry;

public:
  bitmap() = default;

  bitmap(const size &size);
  bitmap(const vect &data, const size s);
  bitmap(const bmp_image &);

  rect geometry() const;
  vect &raw_data();
  const vect &raw_data() const;
  uint bytes_per_row() const;

  bool pixel(const point &p) const;
  bool pixel(uint x, uint y) const;

  void draw_pixel(const point &p, bool white = false);
  void draw_pixel(uint x, uint y, bool white = false);

  void crop(const rect &area);
  bitmap cropped(const rect &area) const;
};

} // namespace ui
