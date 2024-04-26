#pragma once

#include "spi.h"

#include <bitmap.h>
#include <byte_util.h>
#include <rect.h>

#include <gpiod.hpp>

#define WAVESHARE_EPD_WIDTH 122
#define WAVESHARE_EPD_HEIGHT 250

/**
 * By github.com/BortEngineerDude
 *
 * The Waveshare E-ink display V4 driver class.
 * Meant to be used to control the E-Ink display of this parts:
 * Waveshare SKUs 19493, 20716: "2.13inch Touch e-Paper HAT"
 * https://www.waveshare.com/2.13inch-touch-e-paper-hat.htm
 * https://www.waveshare.com/2.13inch-touch-e-paper-hat-with-case.htm
 *
 * Has some tweaks under the hood to make coordinates on the display
 * compatible with coordinates that are provided by the bundled touch panel AND
 * to make display coordinate system organized like one could expect from
 * any general purpose display, i.e.:
 * - top left pixel is at x=0, y=0;
 * - Incremening X will lead towards the right side of the display;
 * - Incremening Y will lead towards the bottom side of the display.
 *
 * NOTE: the device has the portrait orientation.
 */
class waveshare_eink : public spi::device {
public:
  enum refresh_mode { full, patrial };
  enum auto_refresh_mode { none, full_refresh, clear_to_white };

  using path = std::filesystem::path;
  using byte = bytes::byte;
  using vect = bytes::vect;
  using rect = geometry::rect;
  using line = gpiod::line;

private:
  enum command : byte;

  line m_reset;
  line m_data_command;
  line m_busy;

  rect m_geometry = {0, 0, WAVESHARE_EPD_WIDTH - 1, WAVESHARE_EPD_HEIGHT - 1};

  refresh_mode m_refresh_mode = patrial;
  auto_refresh_mode m_auto_full_refresh = none;
  uint m_patrial_refreshes = 0;
  uint m_max_part_refreshes = 5;
  bool m_auto_refreshing = false;

  /**
   * Perform a hard reset followed by a soft reset.
   */
  void reset();

  /**
   * Configure display for patrial or full refresh.
   * @param m set refresh mode
   */
  void apply_config(refresh_mode m);

  /**
   * Send the command byte followed by the command argument as a data.
   * @param a command command to send
   * @param arg a data to send after command
   */
  void send_command(const byte command, const vect &arg = vect());
  void send_command(const byte command, const byte arg);

  /**
   * Send the data as the continuous stream of bytes.
   * @param data the data to send
   */
  void send_data(const vect &data);
  void send_data(const char *data, uint byte_cnt);

  /**
   * Send a single byte of data.
   * @param data a byte to send
   */
  void send_data(byte data);

  void set_draw_region(uint16_t start_x, uint16_t start_y, uint16_t end_x,
                       uint16_t end_y);
  void set_draw_offset(uint16_t x, uint16_t y);

  void pre_upload();
  void post_upload();
  void refresh_display(refresh_mode m);

public:
  waveshare_eink(const path &p, line &&reset, line &&data_command, line &&busy);
  ~waveshare_eink();

  void set_auto_refresh(auto_refresh_mode m);
  void set_max_part_updates(uint);

  void set_display_size(int width, int height);
  void set_refresh_mode(refresh_mode m);

  rect geometry() const;
  void wait_for_busy(std::chrono::nanoseconds timeout = {});
  void clear(bool clear_to_black = false);
  void set_raw_framebuffer(const vect &framebuffer);
  void put_bitmap(ui::bitmap b, const geometry::point &to = {});

  void power_off();
};
