#include "touch.h"

bool event::differs(const event &other) const {
  return touch_id != other.touch_id || x != other.x || y != other.y ||
         size != other.size;
}
