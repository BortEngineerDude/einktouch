// By gh/BortEngineerDude
#pragma once

#include "i2c.h"

#include <touch.h>

#include <chrono>
#include <gpiod.hpp>
#include <unordered_map>
#include <vector>

class gt1158 : public i2c::peripheral {
public:
  struct config {
    uint8_t max_touch_points = 0;
    uint16_t max_x = 0;
    uint16_t max_y = 0;
  };

  using event_map = std::unordered_map<uint8_t, event>;

private:
  gpiod::line m_interrupt_line;
  gpiod::line m_reset_line;

  event_map m_touch_track;
  config m_config;
  bool m_locked = false;
  bool m_inverted = true;

  void read_config();

public:
  gt1158(i2c::controller::ptr controller, gpiod::line &&interrupt,
         gpiod::line &&reset);

  void init();
  void reset();

  /**
   * Lock the touch panel using the appropriate hardware feature.
   * Double-tapping the touch panel quickly will generate an unlock event. Use
   * the gt1158::reset function to leave this mode without triggering an unlock
   * event.
   */
  void lock();

  void set_inverted(bool inverted);

  std::string get_id();
  config get_config();

  bool wait_for_events(const std::chrono::nanoseconds &timeout);
  std::vector<event> get_events();
};
