#include <thread>

#include "waveshare_eink.h"
#include <math_util.h>

/**
 * NOTE: nothing about this E-Ink works as one could expect. If you can make
 * some sense out of it, reference documentation can be found in 'doc' dir of
 * this repo.
 */

using namespace bytes;

enum waveshare_eink::command : byte {
  driver_output_control = 0x01,
  deep_sleep = 0x10,
  data_input_mode = 0x11,
  soft_reset = 0x12,
  temperature_source = 0x18,
  begin_update = 0x20,
  display_update_control_1 = 0x21,
  display_update_control_2 = 0x22,
  upload_image_data = 0x24,
  border_waveform_control = 0x3c,
  draw_region_x = 0x44,
  draw_region_y = 0x45,
  draw_offset_x = 0x4e,
  draw_offset_y = 0x4f,
}; // namespace command

void waveshare_eink::reset() {
  const auto pause = std::chrono::milliseconds(20);

  m_reset.set_value(1);
  std::this_thread::sleep_for(pause);
  m_reset.set_value(0);
  std::this_thread::sleep_for(pause);
  m_reset.set_value(1);
  std::this_thread::sleep_for(pause);
  wait_for_busy();

  send_command(command::soft_reset);
  wait_for_busy();
}

void waveshare_eink::send_command(const byte command, const vect &arg) {
  m_data_command.set_value(0);
  write(command);

  if (arg.empty())
    return;

  m_data_command.set_value(1);
  write(arg);
}

void waveshare_eink::send_command(const byte command, const byte arg) {
  m_data_command.set_value(0);
  write(command);

  m_data_command.set_value(1);
  write(arg);
}

void waveshare_eink::send_data(const vect &data) {
  m_data_command.set_value(1);
  write(data);
}

void waveshare_eink::send_data(const char *data, uint byte_cnt) {
  m_data_command.set_value(1);
  write(data, byte_cnt);
}

void waveshare_eink::send_data(const byte data) {
  m_data_command.set_value(1);
  write(le(data));
}

waveshare_eink::waveshare_eink(const path &p, line &&reset, line &&data_command,
                               line &&busy)
    : spi::device(p), m_reset(reset), m_data_command(data_command),
      m_busy(busy) {
  open();

  gpiod::line_request config;

  // configure outputs
  config.consumer = "Line";
  config.request_type = gpiod::line_request::DIRECTION_OUTPUT;
  config.flags = 0;
  m_reset.request(config);
  m_data_command.request(config); // high for data, low for command

  // configure busy input
  config.request_type = gpiod::line_request::DIRECTION_INPUT |
                        gpiod::line_request::EVENT_FALLING_EDGE;
  config.flags = gpiod::line_request::FLAG_BIAS_PULL_DOWN;
  m_busy.request(config);

  set_bits_per_word(8);
  set_mode(spi::chip_select::active_low, spi::bit_order::msb_first,
           spi::bus_mode::four_wire, spi::spi_mode::_0);
  set_bus_frequency(20'000'000);
  set_data_delay(0);

  apply_config(m_refresh_mode);
}

waveshare_eink::~waveshare_eink() { power_off(); }

void waveshare_eink::set_auto_refresh(auto_refresh_mode m) {
  m_auto_full_refresh = m;
}

void waveshare_eink::set_max_part_updates(uint updates) {
  m_max_part_refreshes = updates;
}

void waveshare_eink::apply_config(refresh_mode m) {
  reset();

  // send_command(command::data_input_mode, 0x00);
  send_command(command::data_input_mode, 0x00);
  send_command(command::driver_output_control, {0xf9, 0, 0});

  // Data input mode: autodecrement both X and Y registers, X will be
  // decremented first, Y will be decremented after each X underflow.
  switch (m) {
  case refresh_mode::full:
    send_command(command::border_waveform_control, 0x05);
    send_command(command::display_update_control_1, {0, 0x80});
    send_command(command::temperature_source, 0x80);
    break;

  case refresh_mode::patrial:
    send_command(command::border_waveform_control, 0x80);
    break;
  }

  wait_for_busy();
}

geometry::rect waveshare_eink::geometry() const { return m_geometry; }

void waveshare_eink::set_display_size(int width, int height) {
  m_geometry.set_size({width - 1, height - 1});
}

void waveshare_eink::set_refresh_mode(refresh_mode m) {
  m_refresh_mode = m;
  if (m_refresh_mode == refresh_mode::full)
    m_patrial_refreshes = 0;

  apply_config(m);
}

void waveshare_eink::clear(bool clear_to_black) {
  auto size = m_geometry.size();
  auto row_size = div_ceil(size.width(), 8);

  vect data(size.height() * row_size, clear_to_black ? 0 : 0xff);
  set_raw_framebuffer(data);
}

void waveshare_eink::set_raw_framebuffer(const vect &framebuffer) {
  pre_upload();

  auto size = m_geometry.size();
  set_draw_region(0, 0, size.width(), size.height());
  set_draw_offset(0, 0);

  send_command(command::upload_image_data, framebuffer);

  post_upload();
}

void waveshare_eink::put_bitmap(ui::bitmap b, const geometry::point &to) {
  pre_upload();

  auto geometry = b.geometry();
  geometry.set_position(to);

  if (!m_geometry.contains(geometry)) {
    auto trim_to = m_geometry.overlap(geometry);
    trim_to.set_position(0, 0);
    b.crop(trim_to);
    geometry = b.geometry();
    geometry.set_position(to);
  }

  auto [disp_width, disp_height] = m_geometry.size().dimensions();
  auto [width, height] = geometry.size().dimensions();
  auto [x, y] = geometry.pos().coords();

  set_draw_region(x, y, width, height);

  auto data = b.raw_data();
  auto bytes_per_row = b.bytes_per_row();

  for (uint row = 0; row < height; ++row) {
    set_draw_offset(disp_width - 2 - x, disp_height - row - y);
    send_command(command::upload_image_data);
    send_data(reinterpret_cast<char *>(data.data() + row * bytes_per_row),
              bytes_per_row);
  }

  post_upload();
}

void waveshare_eink::pre_upload() {
  if (m_refresh_mode == refresh_mode::full ||
      m_auto_full_refresh == auto_refresh_mode::none || m_auto_refreshing)
    return;

  if (m_patrial_refreshes < m_max_part_refreshes)
    return;

  m_auto_refreshing = true;

  if (m_auto_full_refresh == auto_refresh_mode::full_refresh) {
    apply_config(refresh_mode::full);
    return;
  }

  clear();
}

void waveshare_eink::post_upload() {
  if (m_refresh_mode == refresh_mode::full) {
    refresh_display(refresh_mode::full);
    return;
  }

  if (m_auto_full_refresh == auto_refresh_mode::none) {
    refresh_display(refresh_mode::patrial);
    return;
  }

  if (m_auto_full_refresh == auto_refresh_mode::full_refresh &&
      m_auto_refreshing) {
    refresh_display(refresh_mode::full);
    apply_config(refresh_mode::patrial);
    m_patrial_refreshes = 0;
    m_auto_refreshing = false;
    return;
  }

  if (m_auto_full_refresh == auto_refresh_mode::clear_to_white &&
      m_auto_refreshing) {
    refresh_display(refresh_mode::patrial);
    m_patrial_refreshes = 0;
    m_auto_refreshing = false;
    return;
  }

  refresh_display(refresh_mode::patrial);
  ++m_patrial_refreshes;
}

void waveshare_eink::refresh_display(refresh_mode m) {
  byte mode_payload = 0xff;
  if (m == refresh_mode::full)
    mode_payload = 0xf7;

  send_command(command::display_update_control_2, mode_payload);
  send_command(command::begin_update);
  wait_for_busy();
}

void waveshare_eink::wait_for_busy(std::chrono::nanoseconds timeout) {
  if (m_busy.get_value()) { // if the EInk display is currently busy...
    auto begin = std::chrono::steady_clock::now();
    m_busy.event_read_multiple(); // clear any pending events
    m_busy.event_wait(timeout);   // wait for a falling edge event
    m_busy.event_read_multiple(); // consume all new events
    auto duration = std::chrono::steady_clock::now() - begin;
    std::cout << "Spent "
              << std::chrono::duration_cast<std::chrono::milliseconds>(duration)
              << " ms busy waiting" << std::endl;
  }
}

void waveshare_eink::set_draw_region(uint16_t start_x, uint16_t start_y,
                                     uint16_t end_x, uint16_t end_y) {
  send_command(command::draw_region_x);
  send_data((end_x >> 3) & 0xff);
  send_data((start_x >> 3) & 0xff);

  send_command(command::draw_region_y);
  send_data(le(end_y));
  send_data(le(start_y));
}

void waveshare_eink::set_draw_offset(uint16_t x, uint16_t y) {
  send_command(command::draw_offset_x, static_cast<byte>((x >> 3) & 0xff));
  send_command(command::draw_offset_y, le(y));
}

void waveshare_eink::power_off() { send_command(command::deep_sleep, 1); }
