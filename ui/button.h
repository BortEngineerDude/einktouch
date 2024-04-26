#include "element.h"
#include <functional>

namespace ui {

class button : protected element {
  using rect = geometry::rect;

  bool m_toggleable = false;
  bool m_toggled = false;
  bool m_waiting_for_release = false;

  std::function<void(void)> m_on_clicked_callback;
  std::function<void(bool)> m_on_toggled_callback;

protected:
  virtual void draw();
  virtual bool on_touch_event(const event &ev);

public:
  button(const rect &relative_geometry, element *superelement = nullptr);
  button(rect &&relative_geometry, element *superelement = nullptr);
  virtual ~button();

  bool toggleable() const;
  bool toggled() const;

  void set_toggleable(bool);
  void set_toggled(bool);

  void set_clicked_callback(std::function<void(void)> func);
  void set_toggled_callback(std::function<void(bool)> &&func);
};

} // namespace ui
