#pragma once

#include "point.h"

namespace geometry {

class line {
  point m_begin;
  point m_end;

  mutable bool m_coeffs_cached = false;
  mutable double m_slope = 0;
  mutable double m_intercept = 0;
  void calculate_slope_intercept() const;

public:
  line() = default;
  line(int x1, int y1, int x2, int y2);
  line(const point &begin, const point &end);

  void set_begin(int x, int y);
  void set_begin(const point &p);
  void set_end(int x, int y);
  void set_end(const point p);

  point begin() const;
  point end() const;
  std::pair<point &, point &> points_ref();
  std::pair<const point &, const point &> points_ref() const;

  double slope() const;
  double intercept() const;
  std::pair<double, double> coefficients() const;

  bool intersects(const line &other) const;
  point intersection(const line &other) const;

  int resolve_y(int x) const;
  int resolve_x(int y) const;
};

} // namespace geometry

std::istream &operator>>(std::istream &stream, geometry::line &l);
std::ostream &operator<<(std::ostream &stream, const geometry::line &l);
