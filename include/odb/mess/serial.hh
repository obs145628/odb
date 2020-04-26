//===-- mess/serial.hh - Serialization system -------------------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Several utils to perform arch-independant data seralization
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cassert>
#include <cstring>
#include <vector>

#include "fwd.hh"

namespace odb {

template <class T> void sb_serialize(SerialOutBuff &os, const T &item);

template <class T> void sb_unserialize(SerialInBuff &is, T &item);

/// Output stream with a bytes array
/// Serialized object are written to this array
class SerialOutBuff {
public:
  /// Create an empty buffer
  SerialOutBuff() = default;

  /// Reset to empty buffer
  void reset() { _data.clear(); }

  /// Write len bytes in data to output buffer
  void write(const char *data, std::size_t len) {
    _data.insert(_data.end(), data, data + len);
  }

  /// Methods used once the serialization is complete to access the data
  const char *get_data() const { return &_data[0]; }
  std::size_t get_size() const { return _data.size(); }

private:
  std::vector<char> _data;
};

/// Input stream with a bytes array
/// Objects are unserialized from this array
class SerialInBuff {
public:
  /// Unitialized empty inbuff
  SerialInBuff() { reset(0); }

  /// Create an input buffer with `len` uninitialized bytes
  SerialInBuff(std::size_t len) { reset(len); }

  /// Create an input buffer containing len bytes in data
  /// Data is copied
  SerialInBuff(const char *data, std::size_t len) { reset(data, len); }

  /// Reset binary content to a new unitialized buffer of `len` bytes
  /// Also reset read position to 0
  void reset(std::size_t len) {
    _data.resize(len);
    _pos = 0;
  }

  /// Reset binary content to buffer with `bytes` from `data`
  /// Also reset read position to 0
  void reset(const char *data, std::size_t len) {
    _data.assign(data, data + len);
    _pos = 0;
  }

  /// Read len bytes from buffer to out_data
  /// Panic if there is less than len available bytes
  void read(char *out_data, std::size_t len) {
    assert(_pos + len <= _data.size());
    std::memcpy(out_data, &_data[_pos], len);
    _pos += len;
  }

  /// Should be called after the unserialization is complete
  /// Panic if there is some bytes left unread
  void check_eof() { assert(_pos == _data.size()); }

  /// Methods used before the serialization is complete to set the data
  char *get_data() { return &_data[0]; }
  std::size_t get_size() const { return _data.size(); }

private:
  std::vector<char> _data;
  std::size_t _pos;
};

template <class T> SerialOutBuff &operator<<(SerialOutBuff &os, const T &item) {
  sb_serialize(os, item);
  return os;
}

template <class T> SerialInBuff &operator>>(SerialInBuff &is, T &item) {
  sb_unserialize(is, item);
  return is;
}

template <class T> void sb_serial_raw(SerialOutBuff &os, const T &item) {
  os.write(reinterpret_cast<const char *>(&item), sizeof(item));
}

template <class T> T sb_unserial_raw(SerialInBuff &is) {
  T res;
  is.read(reinterpret_cast<char *>(&res), sizeof(res));
  return res;
}

} // namespace odb
