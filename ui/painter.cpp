#include "painter.h"

namespace ui {

painter::painter(bitmap &b, const rect &draw_area)
    : m_bitmap(b), m_translated_geometry(b.geometry().overlap(draw_area)),
      m_geometry(m_translated_geometry) {
  m_geometry.set_position({});
}

void painter::set_point_style(point_style_t::shape_t shape, uint radius,
                              bool white) {
  m_point_style = {shape, radius, white};
}

void painter::set_point_style(point_style_t style) { m_point_style = style; }

point_style_t painter::point_style() const { return m_point_style; }

bool painter::pixel(const point &p) const { return m_bitmap.pixel(p); }

bool painter::pixel(uint x, uint y) const { return m_bitmap.pixel(x, y); }

void painter::draw_pixel(const point &p, bool white) {
  m_bitmap.draw_pixel(p + m_translated_geometry.top_left(), white);
}

void painter::draw_pixel(uint x, uint y, bool white) {
  auto [translated_x, translated_y] = m_translated_geometry.top_left().coords();
  m_bitmap.draw_pixel(translated_x + x, translated_y + y, white);
}

void painter::draw_filled_rect(const rect &r) {
  auto bounds = m_geometry.overlap(r);
  auto [min_x, min_y] = bounds.top_left().coords();
  auto [max_x, max_y] = bounds.bottom_right().coords();

  for (int y = min_y; y < max_y; ++y)
    for (int x = min_x; x < max_x; ++x)
      draw_pixel(x, y, m_point_style.white);
}

void painter::draw_filled_rect(int x1, int y1, int x2, int y2) {
  draw_filled_rect({point(x1, y1), point(x2, y2)});
}

void painter::draw_rect_outline(const rect &r) {
  auto vertices = r.vertices();

  for (int edge = 0; edge < vertices.size() - 1; ++edge)
    draw_line(vertices[edge], vertices[edge + 1]);
  draw_line(vertices[vertices.size() - 1], vertices[0]);
}

void painter::draw_rect_outline(int x1, int y1, int x2, int y2) {
  draw_rect_outline({point(x1, y1), point(x2, y2)});
}

void painter::draw_filled_circle(const point &p, uint radius) {
  draw_filled_circle(p.x(), p.y(), radius);
}

void painter::draw_filled_circle(int x, int y, uint radius) {
  int radius_squared = radius * radius;
  int radius_doubled = radius << 1; // var << 1 == var / 2;
  int area = radius_squared << 2;

  for (int i = 0; i < area; i++) {
    auto div = std::div(i, radius_doubled);
    int delta_x = div.rem - radius;
    int delta_y = div.quot - radius;

    if (delta_x * delta_x + delta_y * delta_y < radius_squared)
      draw_pixel(x + delta_x, y + delta_y, m_point_style.white);
  }
}

void painter::draw_point(const point &p) {
  switch (m_point_style.shape) {
  case point_style_t::round: {
    draw_filled_circle(p, m_point_style.radius);
    break;
  }

  case point_style_t::square: {
    auto [begin_x, begin_y] = p.coords();

    int end_x = begin_x + m_point_style.radius;
    int end_y = begin_y + m_point_style.radius;
    begin_x -= m_point_style.radius;
    begin_y -= m_point_style.radius;
    draw_filled_rect(begin_x, begin_y, end_x, end_y);
    break;
  }
  }
}

void painter::draw_line(const line &line) {
  const auto [begin, end] = line.points_ref();
  auto [x1, y1] = begin.coords();
  auto [x2, y2] = end.coords();

  if (std::abs(x1 - x2) > std::abs(y1 - y2)) {
    auto direction = x1 < x2 ? 1 : -1;
    for (int x = x1; x != x2; x += direction) {
      const point p{x, line.resolve_y(x)};
      if (m_geometry.contains(p))
        draw_point(p);
    }
  } else {
    auto direction = y1 < y2 ? 1 : -1;
    for (int y = y1; y != y2; y += direction) {
      const point p{line.resolve_x(y), y};
      if (m_geometry.contains(p))
        draw_point(p);
    }
  }
}

void painter::draw_line(const point &start, const point &end) {
  draw_line({start, end});
}

void painter::draw_line(int x1, int y1, int x2, int y2) {
  draw_line({
      x1,
      y1,
      x2,
      y2,
  });
}

} // namespace ui
