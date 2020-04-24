//===-- mess/message.hh - Message class definition --------------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Message class, generic low-overhead data type to serialize data
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>

#define ODB_MESSAGE_STRUCT struct __attribute__((__packed__))

namespace odb {

/// Message is a wrapper around a bytes buffer that is used so send / receive
/// messages through serialization, using raw bytes representation
/// One message object should be reused again and again, to reduce memory
/// allocation
class Message {

public:
  using typeid_t = std::uint8_t;

private:
  struct __attribute__((__packed__)) Header {
    typeid_t type;
    std::uint16_t size;
  };

public:
  static constexpr std::size_t DEFAULT_ALLOC = 1;
  static constexpr std::size_t HEADER_SIZE = sizeof(Header);

public:
  /// Create a message with no data (empty)
  Message()
      : _alloc_buff(nullptr), _buff(&_local_buff[0]), _size(0),
        _cap(DEFAULT_ALLOC) {}

  ~Message() { delete[] _alloc_buff; }

  Message(const Message &) = delete;

  /// Allocate memory only for the header
  /// Used when needed to read the header
  /// Must read HEADER_SIZE bytes to returned pointer
  /// Leave object in invalid state until resolved with `alloc_generic`
  char *alloc_header() {
    assert(empty());
    _resize(HEADER_SIZE);
    return &_buff[0];
  }

  /// Used when neded to read message
  /// Must be called after alloc_header() and header stored
  /// Alloc required space for the message, and returns pointer to allocated
  /// data Store message size in bytes in `mess_size`
  char *alloc_generic(std::size_t &mess_size) {
    mess_size = _get_header_uncheck().size;
    _resize(HEADER_SIZE + mess_size);
    return &_buff[HEADER_SIZE];
  }

  // Access to binary data of message
  // only valid if not empty
  const char *data() const {
    assert(!empty());
    return &_buff[0];
  }
  char *data() {
    assert(!empty());
    return &_buff[0];
  }
  std::size_t data_size() const {
    assert(!empty());
    return _size;
  }

  /// Destroy message if there is one
  void reset() { _resize(0); }

  /// @returns true if there is a message
  bool empty() const { return _size == 0; }

  /// Get current message type
  std::uint16_t get_type() const {
    assert(!empty());
    return _get_header_uncheck().type;
  }

  /// Allocate space for a new message of a specific type
  /// `extra_bytes` is used for variable-size structs, it allocates the size
  /// struct + this value
  template <class MessageType>
  MessageType &alloc_as(std::size_t extra_bytes = 0) {
    assert(empty());
    _resize(HEADER_SIZE + sizeof(MessageType) + extra_bytes);

    auto &header = _get_header_uncheck();
    header.type = MessageType::MESSAGE_TYPEID;
    header.size = sizeof(MessageType) + extra_bytes;
    return *_get_as_uncheck<MessageType>();
  }

  template <class MessageType> const MessageType &get_as() const {
    assert(get_type() == MessageType::MESSAGE_TYPEID);
    return *_get_as_uncheck<MessageType>();
  }

private:
  char _local_buff[DEFAULT_ALLOC];
  char *_alloc_buff; // pointer to optionally allocated buffer
  char *_buff;       // Pointer to used data buffer
  std::size_t _size; // 0 if empty, or size header, or size header + size mess
  std::size_t _cap;

  void _resize(std::size_t new_size) {
    std::size_t new_cap = _cap;
    while (new_cap <= new_size)
      new_cap *= 2;

    if (new_cap > _cap) {
      char *new_buf = new char[new_cap];
      std::memcpy(new_buf, _buff, _size);
      _cap = new_cap;

      delete[] _alloc_buff;
      _alloc_buff = new_buf;
      _buff = new_buf;
    }

    _size = new_size;
  }

  Header &_get_header_uncheck() { return *reinterpret_cast<Header *>(data()); }

  const Header &_get_header_uncheck() const {
    return *reinterpret_cast<const Header *>(data());
  }

  template <class T> T *_get_as_uncheck() {
    return reinterpret_cast<T *>(data() + sizeof(Header));
  }

  template <class T> const T *_get_as_uncheck() const {
    return reinterpret_cast<const T *>(data() + sizeof(Header));
  }
};

} // namespace odb
