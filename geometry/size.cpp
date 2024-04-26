#include "size.h"

#include <stdexcept>

namespace geometry {

size::size(int w, int h) : m_width(w), m_height(h) {}

int &size::width_ref() { return m_width; }

int &size::height_ref() { return m_height; }

std::pair<int &, int &> size::ref() { return {m_width, m_height}; }

int size::width() const { return m_width; }

int size::height() const { return m_height; }

std::pair<int, int> size::dimensions() const { return {m_width, m_height}; }

void size::set_width(int width) { m_width = width; }

void size::set_height(int height) { m_height = height; }

void size::set_size(int width, int height) {
  m_width = width;
  m_height = height;
}

unsigned size::area() const { return std::abs(m_width) * std::abs(m_height); }

} // namespace geometry

std::istream &operator>>(std::istream &stream, geometry::size &s) {
  static const std::string stream_mark = "size=[";

  std::string str_input;
  stream >> str_input;

  if (str_input != stream_mark)
    throw std::invalid_argument("Unexpected input");

  stream >> s.width_ref();
  stream >> str_input;

  if (str_input != "x")
    throw std::invalid_argument("Unexpected input");

  stream >> s.height_ref();
  stream >> str_input;

  if (str_input != "]")
    throw std::invalid_argument("Unexpected input");

  return stream;
}

std::ostream &operator<<(std::ostream &stream, const geometry::size &s) {
  stream << "size=[ " << s.width() << " x " << s.height() << " ]";
  return stream;
}
