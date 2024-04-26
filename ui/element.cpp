#include "element.h"

namespace ui {

element::element(const rect &relative_geometry, element *superelement)
    : m_relative_geometry(relative_geometry), m_superelement(superelement) {

  if (m_superelement)
    m_superelement->m_subelements.emplace_back(this);
  else
    m_bitmap.reset(new bitmap(relative_geometry.size()));
}

element::element(rect &&relative_geometry, element *superelement)
    : m_relative_geometry(relative_geometry), m_superelement(superelement) {
  if (m_superelement)
    m_superelement->m_subelements.emplace_back(this);
  else
    m_bitmap.reset(new bitmap(relative_geometry.size()));
}

element::~element() {
  for (auto e : m_subelements)
    delete e;
}

geometry::rect element::geometry() {
  auto result = m_relative_geometry;
  result.set_position(0, 0);
  return result;
}

std::pair<element *, geometry::rect> element::geometry_relative_to_root() {
  auto super = this;
  auto geometry = m_relative_geometry;

  while (super) {
    if (super->m_superelement) {
      super = super->m_superelement;
      auto translate_point = super->m_relative_geometry.pos();
      geometry.move(translate_point.x(), translate_point.y());
    } else
      break;
  }

  return {super, geometry};
}

std::pair<element *, geometry::rect> element::geometry_relative_to_parent() {
  return {m_superelement, m_relative_geometry};
}

void element::render_all() {
  draw();

  auto current = m_subelements.rbegin();
  auto end = m_subelements.rend();

  while (current != end) {
    (*current)->render_all();
    ++current;
  }
}

void element::draw() {
  auto painter = get_painter();
  auto geometry = element::geometry();

  painter->set_point_style(point_style_t::square, 1, true);
  painter->draw_filled_rect(geometry);

  painter->set_point_style(point_style_t::square, 3, false);
  painter->draw_rect_outline(geometry);
}

std::shared_ptr<painter> element::get_painter() {
  auto [root, geometry] = geometry_relative_to_root();
  return std::make_shared<painter>(*root->m_bitmap.get(), geometry);
}

bool element::on_touch_event(const event &ev) { return false; }

element *element::get_superelement() { return m_superelement; }

element *element::get_root_element() {
  if (!m_superelement)
    return this;

  auto super = m_superelement;
  while (super)
    if (super->m_superelement)
      super = super->m_superelement;
    else
      break;

  return super;
}

bitmap &element::get_bitmap() { return *get_root_element()->m_bitmap.get(); }

bool element::process_event(const std::any &ev) {
  if (ev.type() != typeid(event) || !ev.has_value())
    return false;

  for (auto sub : m_subelements)
    if (sub->process_event(ev))
      return true;

  event e = any_cast<event>(ev);
  auto [element, geometry] = geometry_relative_to_root();

  if (geometry.contains({e.x, e.y}))
    if (on_touch_event(e)) {
      render_all();
      return true;
    }

  return false;
}

} // namespace ui
