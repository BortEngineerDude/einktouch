// By gh/BortEngineerDude

#include <button.h>
#include <gt1158.h>
#include <i2c.h>
#include <waveshare_eink.h>

#include <chrono>
#include <gpiod.hpp>
#include <iostream>

std::string timestamp() {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
  return ss.str();
}

int main() {
  i2c::fs::path touch_dev = "/dev/i2c-1";
  spi::fs::path display_dev = "/dev/spidev0.0";
  gpiod::chip gpio("0");

  auto i2c_controller = std::make_shared<i2c::controller>(touch_dev);
  i2c_controller->open();

  gt1158 touchscreen(i2c_controller, gpio.get_line(118), gpio.get_line(32));
  waveshare_eink eink(display_dev, gpio.get_line(117), gpio.get_line(65),
                      gpio.get_line(110));

  const std::chrono::seconds poll_duration(1);

  ui::element root(eink.geometry(), nullptr);
  auto s1 = new ui::element({10, 10, 100, 90}, &root);
  auto b11 = new ui::button({10, 10, 80, 30}, s1);
  auto b12 = new ui::button({10, 50, 80, 30}, s1);

  b11->set_toggleable(true);
  b12->set_toggleable(true);

  auto s2 = new ui::element({10, 110, 100, 135}, &root);
  auto b21 = new ui::button({10, 10, 80, 30}, s2);
  auto b22 = new ui::button({10, 50, 80, 30}, s2);
  auto b23 = new ui::button({10, 90, 80, 30}, s2);

  root.render_all();
  eink.clear();
  eink.put_bitmap(root.get_bitmap());

  bool running = true;
  waveshare_eink::refresh_mode refresh = waveshare_eink::refresh_mode::patrial;
  waveshare_eink::auto_refresh_mode auto_refresh =
      waveshare_eink::auto_refresh_mode::none;

  bool need_update = false;

  auto clicked = []() { std::cout << "Clicked" << std::endl; };
  auto toggled = [](bool toggled) {
    std::cout << "Toggled: " << std::boolalpha << toggled << std::endl;
  };
  auto switch_refresh_mode = [&eink, &auto_refresh, &root]() {
    std::cout << "Auto refresh mode: ";

    switch (auto_refresh) {
    case waveshare_eink::auto_refresh_mode::none:
      auto_refresh = waveshare_eink::auto_refresh_mode::full_refresh;
      std::cout << "full\n";
      break;

    case waveshare_eink::auto_refresh_mode::full_refresh:
      auto_refresh = waveshare_eink::auto_refresh_mode::clear_to_white;
      std::cout << "clear to white\n";
      break;

    case waveshare_eink::auto_refresh_mode::clear_to_white:
      auto_refresh = waveshare_eink::auto_refresh_mode::none;
      std::cout << "none\n";
      break;
    }

    eink.clear();
    eink.set_auto_refresh(auto_refresh);
    eink.put_bitmap(root.get_bitmap());
  };
  auto exit = [&running]() { running = false; };

  b11->set_toggled_callback(toggled);

  b21->set_clicked_callback(clicked);
  b22->set_clicked_callback(switch_refresh_mode);
  b23->set_clicked_callback(exit);

  while (running) {
    if (touchscreen.wait_for_events(poll_duration)) {
      auto events = touchscreen.get_events();
      if (events.empty())
        continue;

      std::cout << timestamp() << " Got " << events.size() << " events:\n";
      for (const auto &ev : events) {
        std::cout << " " << ev << "\n";

        if (root.process_event(ev))
          need_update = true;
      }

      if (need_update) {
        need_update = false;
        eink.put_bitmap(root.get_bitmap());
      }

      std::cout << std::endl;
    }
  }

  eink.clear();
  return EXIT_SUCCESS;
}
