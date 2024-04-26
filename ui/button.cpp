#include "button.h"

namespace ui {

void button::draw() {
  element::draw();

  if (m_toggleable && m_toggled) {
    auto p = get_painter();
    auto g = geometry();

    g.move(5, 5);
    g.resize(-10, -10);

    p->draw_filled_rect(g);
  }
}

bool button::on_touch_event(const event &ev) {
  switch (ev.type) {
  case event::type_t::touch:
    m_waiting_for_release = true;
    return true;

  case event::type_t::release:
    if (m_waiting_for_release) {
      m_waiting_for_release = false;

      if (m_toggleable) {
        m_toggled = !m_toggled;
        if (m_on_toggled_callback)
          m_on_toggled_callback(m_toggled);

      } else {
        if (m_on_clicked_callback)
          m_on_clicked_callback();
      }
    }

    return true;
  default:
    return false;
  }
}

button::button(const geometry::rect &relative_geometry, element *superelement)
    : element(relative_geometry, superelement) {}

button::button(geometry::rect &&relative_geometry, element *superelement)
    : element(relative_geometry, superelement) {}

button::~button() {}

bool button::toggleable() const { return m_toggleable; }

bool button::toggled() const { return m_toggled; }

void button::set_toggleable(bool toggleable) {
  m_toggleable = toggleable;
  if (!m_toggleable)
    m_toggled = false;
}

void button::set_toggled(bool toggled) {
  if (!m_toggleable)
    return;

  m_toggled = toggled;
  draw();
}

void button::set_clicked_callback(std::function<void(void)> func) {
  m_on_clicked_callback = func;
}

void button::set_toggled_callback(std::function<void(bool)> &&func) {
  m_on_toggled_callback = func;
}

} // namespace ui
