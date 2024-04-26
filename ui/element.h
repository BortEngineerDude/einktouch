#pragma once

#include "bitmap.h"
#include "painter.h"

#include <touch.h>

#include <any>
#include <list>
#include <memory>

namespace ui {

class element {
  using rect = geometry::rect;

  std::unique_ptr<bitmap> m_bitmap;

  rect m_relative_geometry;
  std::list<element *> m_subelements;
  element *m_superelement;

protected:
  virtual void draw();
  virtual bool on_touch_event(const event &ev);

  std::shared_ptr<painter> get_painter();

public:
  element(const rect &relative_geometry, element *superelement = nullptr);
  element(rect &&relative_geometry, element *superelement = nullptr);
  ~element();

  void render_all();

  rect geometry();
  std::pair<element *, rect> geometry_relative_to_root();
  std::pair<element *, rect> geometry_relative_to_parent();

  element *get_superelement();
  element *get_root_element();

  bitmap &get_bitmap();

  bool process_event(const std::any &ev);
};

} // namespace ui
