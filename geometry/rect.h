#pragma once

#include <vector>

#include "point.h"
#include "size.h"

namespace geometry {

/**
 * Rectangle class for displays i.e. x and y describes top left corner; x and y
 * will be both at max at the bottom right corner.
 */
class rect {
  point m_p;
  class size m_s;

public:
  rect() = default;
  rect(int x, int y, int width, int height);
  rect(point start, point end);
  rect(point pos, class size s);

  /**
   * Transform coordinates to make width and height positive values, if
   * necessary.
   */
  void normalize();

  /**
   * Get the size of a rectangle
   */
  class size size() const;

  /**
   * Get the top left corner position (the same as rect::top_left())
   */
  point pos() const;

  /**
   * \defgroup rect_corners
   * Get the position of a corresponding corner.
   * @{
   */
  point top_left() const;
  point top_right() const;
  point bottom_left() const;
  point bottom_right() const;
  /** @} */

  /**
   * Get all 4 verices as vector, starting from top left, clockwise.
   */
  std::vector<point> vertices() const;
  /**
   * \defgroup rect_reposition
   * Set the position of a top left corner.
   * @{
   */
  void set_position(int x, int y);
  void set_position(point p);
  /** @} */

  /**
   * Move top left corner by dx, dy
   */
  void move(int dx, int dy);

  /**
   * \defgroup rect_set_size.
   * Set the rect size.
   * @{
   */
  void set_size(int width, int height);
  void set_size(const class size &s);
  /** @} */

  /**
   * Increase or decrease size by dw and dh
   */
  void resize(int dw, int dh);

  int area() const;
  int perimeter() const;

  /**
   * Does this rectangle contains a point?
   * @param p point to verify
   * @return true if rectangle do contain a point.
   */
  bool contains(point p) const;

  /**
   * Does this rectangle contains another rectangle?
   * @param p point to verify
   * @return true if rectangle do contain a point.
   */
  bool contains(const rect &other) const;

  /**
   * Does this and other rectangle intersect each other?
   * @param other rectangle to verify
   * @return true if rectangles do intersect each other.
   */
  bool intersects(const rect &other) const;

  /**
   * Get rectangle containing common area of both rectangles.
   * @param other rect to make overlap with.
   * @return common rect of both rects, or area of zero area, if two rectangles
   * do not intersect each other.
   */
  rect overlap(const rect &other) const;

  /**
   * Get rectangle covering both of rectangles.
   * @param other rect to make outline for.
   * @return rectangle which contains both of rectangles.
   */
  rect outline(const rect &other) const;
};

} // namespace geometry

std::istream &operator>>(std::istream &stream, geometry::rect &r);
std::ostream &operator<<(std::ostream &stream, const geometry::rect &r);
