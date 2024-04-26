// By gh/BortEngineerDude
#pragma once
#include <byte_util.h>
#include <filesystem>
#include <memory>

// A wrapper for a Linux I2C "adapter file".

namespace i2c {

using namespace bytes;
namespace fs = std::filesystem;

class peripheral;

class controller {
  friend class i2c::peripheral;

  fs::path m_file_path;
  int m_file_descriptor = -1;

  void write(const byte devaddr, const vect &reg, const vect &payload);
  bytes::vect read(const byte devaddr, const vect &reg, const uint bytes);

public:
  using ptr = std::shared_ptr<i2c::controller>;
  controller(const fs::path &device);
  ~controller();

  void open();
  bool is_open();
  void close();
};

class peripheral {
protected:
  controller::ptr m_controller;
  byte m_address{0};

  peripheral(controller::ptr, const byte address);
  void write(const vect &reg, const vect &payload);
  vect read(const vect &reg, const uint bytes);
};

} // namespace i2c
