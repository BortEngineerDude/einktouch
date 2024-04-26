#pragma once

#include <ostream>
#include <stdint.h>

struct event {
  enum class type_t : uint8_t { none, touch, drag, release, unlock };

  event() = default;

  type_t type = type_t::none;
  uint8_t touch_id = 0;
  uint16_t x = 0;
  uint16_t y = 0;
  uint16_t size = 0;

  /**
   * Is this event *somewhat* different than the other event?
   * @param other event to compare to
   * @return true, if any member EXCEPT FOR THE TYPE is different than in
   * other
   */
  bool differs(const event &other) const;
  bool operator==(const event &other) const = default;
};

std::ostream &operator<<(std::ostream &stream, const event &ev);
