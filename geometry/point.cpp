#include "point.h"

#include <math.h>

namespace geometry {

int &point::x_ref() { return m_x; }

int &point::y_ref() { return m_y; }

std::pair<int &, int &> point::ref() { return {m_x, m_y}; }

int point::x() const { return m_x; }

int point::y() const { return m_y; }

std::pair<int, int> point::coords() const { return {m_x, m_y}; }

void point::set_x(int x) { m_x = x; }

void point::set_y(int y) { m_y = y; }

void point::set_point(int x, int y) { m_x = x, m_y = y; }

double point::distance(const point &other) const {
  return sqrt(pow(m_x - other.m_x, 2) + pow(m_y - other.m_y, 2));
}

point point::operator+(const point &other) const {
  return {m_x + other.m_x, m_y + other.m_y};
}

point point::operator-(const point &other) const {
  return {m_x - other.m_x, m_y - other.m_y};
}

} // namespace geometry

std::istream &operator>>(std::istream &stream, geometry::point &p) {
  static const std::string stream_mark = "point=(";

  std::string str_input;
  stream >> str_input;

  if (str_input != stream_mark)
    throw std::invalid_argument("Unexpected input");

  stream >> p.x_ref();
  stream >> str_input;

  stream >> p.y_ref();
  stream >> str_input;

  if (str_input != ")")
    throw std::invalid_argument("Unexpected input");

  return stream;
}

std::ostream &operator<<(std::ostream &stream, const geometry::point &p) {
  stream << "point=( " << p.x() << "; " << p.y() << " )";
  return stream;
}
