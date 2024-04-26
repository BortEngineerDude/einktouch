#include "rect.h"

namespace geometry {

void rect::normalize() {
  auto [x, y] = m_p.ref();
  auto [w, h] = m_s.ref();

  if (w < 0) {
    x += w;
    w = -w;
  }

  if (h < 0) {
    y += h;
    h = -h;
  }
}

class size rect::size() const { return m_s; }

point rect::top_left() const { return m_p; }

point rect::top_right() const { return {m_p.x() + m_s.width(), m_p.y()}; }

point rect::bottom_left() const { return {m_p.x(), m_p.y() + m_s.height()}; }

point rect::bottom_right() const {
  return {m_p.x() + m_s.width(), m_p.y() + m_s.height()};
}

std::vector<point> rect::vertices() const {
  return {top_left(), top_right(), bottom_right(), bottom_left()};
}

void rect::set_position(int x, int y) { m_p.set_point(x, y); }

void rect::set_position(point p) { m_p = p; }

void rect::move(int dx, int dy) {
  auto [x, y] = m_p.ref();
  x += dx;
  y += dy;
}

void rect::set_size(int width, int height) { m_s.set_size(width, height); }

void rect::set_size(const class size &s) { m_s = s; }

void rect::resize(int dw, int dh) {
  auto [w, h] = m_s.ref();
  w += dw;
  h += dh;
}

point rect::pos() const { return m_p; }

int rect::area() const { return m_s.area(); }

int rect::perimeter() const { return m_s.width() * 2 + m_s.height() * 2; }

bool rect::contains(point p) const {
  return m_p.x() <= p.x() && (m_p.x() + m_s.width()) >= p.x() &&
         m_p.y() <= p.y() && (m_p.y() + m_s.height()) >= p.y();
}

bool rect::contains(const rect &other) const {
  return contains(other.top_left()) && contains(other.bottom_left());
}

bool rect::intersects(const rect &other) const {
  return (other.m_p.x() + other.m_s.width()) >= m_p.x() &&
         (m_p.x() + m_s.width()) >= other.m_p.x() &&
         (other.m_p.y() + other.m_s.height()) >= m_p.y() &&
         (m_p.y() + m_s.height()) >= other.m_p.y();
}

rect rect::overlap(const rect &other) const {
  if (!intersects(other))
    return {};

  auto max_x = std::max(m_p.x(), other.m_p.x());
  auto max_y = std::max(m_p.y(), other.m_p.y());

  return {max_x, max_y,
          std::min(m_p.x() + m_s.width(), other.m_p.x() + other.m_s.width()) -
              max_x,
          std::min(m_p.y() + m_s.height(), other.m_p.y() + other.m_s.height()) -
              max_y};
}

rect rect::outline(const rect &other) const {
  point top_left;
  point bottom_right;

  auto r1 = *this;
  auto r2 = other;

  r1.normalize();
  r2.normalize();

  auto r1_top_left = r1.top_left();
  auto r2_top_left = r2.top_left();

  top_left.x_ref() = std::min(r1_top_left.x(), r2_top_left.x());
  top_left.y_ref() = std::min(r1_top_left.y(), r2_top_left.y());

  auto r1_bottom_right = r1.bottom_right();
  auto r2_bottom_right = r2.bottom_right();

  bottom_right.x_ref() = std::max(r1_bottom_right.x(), r2_bottom_right.x());
  bottom_right.y_ref() = std::max(r1_bottom_right.y(), r2_bottom_right.y());

  return rect(top_left, bottom_right);
}

rect::rect(int x, int y, int width, int height)
    : m_p(x, y), m_s(width, height) {
  normalize();
}

rect::rect(point start, point end) : m_p(start) {
  m_s.set_size(end.x() - m_p.x(), end.y() - m_p.y());

  normalize();
}

rect::rect(point pos, class size s) : m_p(pos), m_s(s) {}

} // namespace geometry

std::istream &operator>>(std::istream &stream, geometry::rect &r) {
  static const std::string stream_mark = "rect={";

  std::string str_input;
  stream >> str_input;

  if (str_input != stream_mark)
    throw std::invalid_argument("Unexpected input");

  geometry::point p;
  geometry::size s;
  stream >> p >> s;

  stream >> str_input;
  if (str_input != "}")
    throw std::invalid_argument("Unexpected input");

  r = geometry::rect(p, s);

  return stream;
}

std::ostream &operator<<(std::ostream &stream, const geometry::rect &r) {
  stream << "rect={ " << r.pos() << " " << r.size() << " }";
  return stream;
}
