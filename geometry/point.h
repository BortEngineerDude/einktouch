#pragma once

#include <iostream>
#include <utility>

namespace geometry {

class point {
  int m_x = 0;
  int m_y = 0;

public:
  point() = default;
  point(int x, int y) : m_x(x), m_y(y) {}

  int &x_ref();
  int &y_ref();
  std::pair<int &, int &> ref();

  int x() const;
  int y() const;
  std::pair<int, int> coords() const;

  void set_x(int x);
  void set_y(int y);
  void set_point(int x, int y);

  double distance(const point &other) const;

  point operator+(const point &other) const;
  point operator-(const point &other) const;
};

} // namespace geometry

std::istream &operator>>(std::istream &stream, geometry::point &p);
std::ostream &operator<<(std::ostream &stream, const geometry::point &p);
