// By gh/BortEngineerDude
#include <ranges>
#include <thread>
#include <unordered_set>

#include "gt1158.h"
#include <byte_util.h>

using namespace bytes;

static const auto lock_control_register = be((uint16_t)0x8040);
static const auto config_register = be((uint16_t)0x8051);
static const auto config_size = 5;
static const auto touch_status_register = be((uint16_t)0x814e);
static const auto lock_status_register = be((uint16_t)0x814c);
static const auto id_register = be((uint16_t)0x8140);
static const auto id_size = 4;

void gt1158::read_config() {
  auto result = read(config_register, config_size);
  buffer parser(result, endian::little);

  parser >> m_config.max_x >> m_config.max_y >> m_config.max_touch_points;
}

gt1158::gt1158(i2c::controller::ptr controller, gpiod::line &&interrupt,
               gpiod::line &&reset)
    : i2c::peripheral(controller, 0x14), m_interrupt_line(interrupt),
      m_reset_line(reset) {
  init();
}

void gt1158::init() {
  gpiod::line_request config;

  config.consumer = "Line";
  config.request_type = gpiod::line_request::EVENT_FALLING_EDGE;
  config.flags = gpiod::line_request::FLAG_BIAS_PULL_UP;
  m_interrupt_line.request(config);

  config.request_type = gpiod::line_request::DIRECTION_OUTPUT;
  config.flags = gpiod::line_request::FLAG_ACTIVE_LOW;
  m_reset_line.request(config);

  gt1158::reset();
  auto id = get_id();
  if (id != "1158") {
    std::stringstream error;
    error << "GT1158: got unexpected id \"" << id << "\"; expected id\"1158\"";
    throw std::runtime_error(error.str());
  }
  read_config();
}

std::string gt1158::get_id() {
  auto result = read(id_register, id_size);
  std::string id(reinterpret_cast<const char *>(result.data()), id_size);
  return id;
}

gt1158::config gt1158::get_config() { return m_config; }

void gt1158::reset() {
  m_locked = false;
  const auto pause = std::chrono::milliseconds(50);
  m_reset_line.set_value(0);
  std::this_thread::sleep_for(pause);
  m_reset_line.set_value(1);
  std::this_thread::sleep_for(pause);
  m_reset_line.set_value(0);
  std::this_thread::sleep_for(pause);
}

void gt1158::lock() {
  // I didn't manage to find a datasheet explaining the meaning of the
  // following code.
  write(lock_control_register, {0x08, 0, 0xf8});
  m_locked = true;

  m_touch_track.clear();
}

void gt1158::set_inverted(bool inverted) { m_inverted = inverted; }

bool gt1158::wait_for_events(const std::chrono::nanoseconds &timeout) {
  return m_interrupt_line.event_wait(timeout);
}

std::vector<event> gt1158::get_events() {
  if (m_interrupt_line.event_read_multiple().empty())
    return {};

  if (m_locked) {
    auto unlock_status = read(lock_status_register, 1);
    if (unlock_status[0] == 0xcc) {
      event ev{};
      ev.type = event::type_t::unlock;
      reset();
      return {ev};
    } else {
      // poll for unlock events
      write(lock_status_register, {0});
      return {};
    }
  }

  auto touch_status = read(touch_status_register, 1)[0];

  if (touch_status) {
    // request coordinates
    write(touch_status_register, {0});

    uint8_t touch_count = touch_status & 0x0f;

    if (touch_count < 1 || touch_count > 5) {
      if (m_touch_track.empty())
        return {};
      else {
        std::vector<event> release_touch;
        release_touch.reserve(m_touch_track.size());
        for (auto &touch : m_touch_track) {
          touch.second.type = event::type_t::release;
          release_touch.emplace_back(touch.second);
        }

        m_touch_track.clear();
        return release_touch;
      }
    }

    auto touches = read(touch_status_register, 1 + touch_count * 8);
    buffer parser(touches, endian::little);
    parser.seek(1); // skip over the first status byte

    std::unordered_set<uint8_t> released_touch_ids;
    std::unordered_set<uint8_t> updated_touch_ids;
    released_touch_ids.reserve(m_touch_track.size());
    for (auto key : std::views::keys(m_touch_track))
      released_touch_ids.insert(key);

    for (int touch_n = 0; touch_n < touch_count; ++touch_n) {
      event ev;
      parser >> ev.touch_id >> ev.x >> ev.y >> ev.size;

      if (m_touch_track.contains(ev.touch_id)) {
        released_touch_ids.erase(ev.touch_id);

        if (m_touch_track[ev.touch_id].differs(ev)) {
          ev.type = event::type_t::drag;
          updated_touch_ids.insert(ev.touch_id);
          m_touch_track[ev.touch_id] = ev;
        }
      } else {
        updated_touch_ids.insert(ev.touch_id);
        ev.type = event::type_t::touch;
        m_touch_track[ev.touch_id] = ev;
      }

      parser.seek(1); // the trailing byte is always 0
    }

    for (auto &id : released_touch_ids) {
      m_touch_track[id].type = event::type_t::release;
      updated_touch_ids.insert(id);
    }

    std::vector<event> result;
    result.reserve(updated_touch_ids.size());
    for (auto id : updated_touch_ids)
      result.emplace_back(m_touch_track[id]);

    std::erase_if(m_touch_track, [&released_touch_ids](const auto &node) {
      return released_touch_ids.contains(node.first);
    });

    return result;
  }

  return {};
}

std::ostream &operator<<(std::ostream &stream, const event &ev) {
  stream << " event={ ";

  switch (ev.type) {
  case event::type_t::none:
    stream << "invalid }";
    return stream;

  case event::type_t::touch:
    stream << "touch";
    break;

  case event::type_t::drag:
    stream << "drag";
    break;

  case event::type_t::release:
    stream << "release";
    break;

  case event::type_t::unlock:
    stream << "unlock }";
    return stream;
  }

  stream << " ID " << (int)ev.touch_id << " ( " << (int)ev.x << " ; "
         << (int)ev.y << " ) size " << (int)ev.size << " }";
  return stream;
}
