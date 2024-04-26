#pragma once

#include "../geometry/line.h"
#include "bitmap.h"

namespace ui {

struct point_style_t {
  enum shape_t { round, square };
  shape_t shape = square;
  uint radius = 1;
  bool white = false;

  point_style_t() = default;
  point_style_t(shape_t shape, uint radius, bool white)
      : shape(shape), radius(radius), white(white) {}
};

class painter {
  using rect = geometry::rect;
  using point = geometry::point;
  using line = geometry::line;

  bitmap &m_bitmap;
  rect m_translated_geometry;
  rect m_geometry;
  point_style_t m_point_style;

public:
  painter(bitmap &b, const rect &draw_area);

  void set_point_style(point_style_t::shape_t shape, uint radius, bool white);
  void set_point_style(point_style_t style);
  point_style_t point_style() const;

  bool pixel(const point &p) const;
  bool pixel(uint x, uint y) const;

  void draw_pixel(const point &p, bool white = false);
  void draw_pixel(uint x, uint y, bool white = false);

  void draw_filled_rect(const rect &r);
  void draw_filled_rect(int x1, int y1, int x2, int y2);

  void draw_rect_outline(const rect &r);
  void draw_rect_outline(int x1, int y1, int x2, int y2);

  void draw_filled_circle(const point &p, uint radius);
  void draw_filled_circle(int x, int y, uint radius);

  void draw_point(const point &p);
  void draw_point(uint x, uint y);

  void draw_line(const line &line);
  void draw_line(const point &start, const point &end);
  void draw_line(int x1, int y1, int x2, int y2);
};

} // namespace ui
