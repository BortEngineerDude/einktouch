#include "line.h"

#include <cmath>
#include <stdexcept>

namespace geometry {

void line::calculate_slope_intercept() const {
  if (m_coeffs_cached)
    return;

  auto [x1, y1] = m_begin.coords();
  auto [x2, y2] = m_end.coords();

  if (x2 - x1) {
    m_slope = static_cast<double>(y2 - y1) / static_cast<double>(x2 - x1);
    m_intercept = static_cast<double>(y1) - m_slope * static_cast<double>(x1);
  } else {
    m_slope = 0;
    m_intercept = x1;
  }

  m_coeffs_cached = true;
}

line::line(int x1, int y1, int x2, int y2) : m_begin(x1, y1), m_end(x2, y2) {}

line::line(const point &begin, const point &end) : m_begin(begin), m_end(end) {}

void line::set_begin(int x, int y) {
  m_coeffs_cached = false;
  m_begin.set_point(x, y);
}

void line::set_begin(const point &p) {
  m_coeffs_cached = false;
  m_begin = p;
}

void line::set_end(int x, int y) {
  m_coeffs_cached = false;
  m_end.set_point(x, y);
}

void line::set_end(const point p) {
  m_coeffs_cached = false;
  m_end = p;
}

point line::begin() const { return m_begin; }

point line::end() const { return m_end; }

std::pair<point &, point &> line::points_ref() { return {m_begin, m_end}; }

std::pair<const point &, const point &> line::points_ref() const {
  return {m_begin, m_end};
}

double line::slope() const {
  calculate_slope_intercept();
  return m_slope;
}

double line::intercept() const {
  calculate_slope_intercept();
  return m_intercept;
}

std::pair<double, double> line::coefficients() const {
  calculate_slope_intercept();
  return {m_slope, m_intercept};
}

bool line::intersects(const line &other) const {
  auto [a, c] = coefficients();
  auto [b, d] = other.coefficients();

  return a != b;
}

point line::intersection(const line &other) const {
  auto [a, c] = coefficients();
  auto [b, d] = other.coefficients();

  if (a == b) {
    if (c == d)
      throw std::invalid_argument("Two lines are the same");
    else
      throw std::invalid_argument("Parallel lines do not intersect each other");
  }

  return {static_cast<int>(std::round((d - c) / (a - b))),
          static_cast<int>(std::round(a * (d - c) / (a - b) + c))};
}

int line::resolve_y(int x) const {
  calculate_slope_intercept();
  return m_slope * x + m_intercept;
}

int line::resolve_x(int y) const {
  calculate_slope_intercept();
  if (m_slope == 0)
    return m_begin.x();

  return (y - m_intercept) / m_slope;
}

} // namespace geometry

std::istream &operator>>(std::istream &stream, geometry::line &l) {
  static const std::string stream_mark = "line={";

  std::string str_input;
  stream >> str_input;

  if (str_input != stream_mark)
    throw std::invalid_argument("Unexpected input");

  auto [b, e] = l.points_ref();

  stream >> b >> e;
  stream >> str_input;

  if (str_input != "}")
    throw std::invalid_argument("Unexpected input");

  return stream;
}

std::ostream &operator<<(std::ostream &stream, const geometry::line &l) {
  stream << "line={ " << l.begin() << " " << l.end() << " }";
  return stream;
}
