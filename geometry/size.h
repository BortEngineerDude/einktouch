#pragma once

#include <iostream>
#include <utility>

namespace geometry {

class size {
  int m_width = 0;
  int m_height = 0;

public:
  size() = default;
  size(int w, int h);

  int &width_ref();
  int &height_ref();
  std::pair<int &, int &> ref();

  int width() const;
  int height() const;
  std::pair<int, int> dimensions() const;

  void set_width(int width);
  void set_height(int height);
  void set_size(int width, int height);

  unsigned area() const;
};

} // namespace geometry

std::istream &operator>>(std::istream &stream, geometry::size &s);
std::ostream &operator<<(std::ostream &stream, const geometry::size &s);
