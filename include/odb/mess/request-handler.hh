//===-- mess/request-handler.hh - RequestHandler class ----------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// RequestHandler class, used to manage serial / unserial of different kind of
/// messages
///
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <vector>

#include "serial.hh"

namespace odb {

class TmpBuffHolder {

public:
  void reset() { _bufs.clear(); }

  template <class T> T *add_buff(std::size_t size) {
    std::size_t buf_size = size * sizeof(T);
    _bufs.push_back(std::make_unique<char[]>(buf_size));
    char *buf = _bufs.back().get();
    T *tbuf = reinterpret_cast<T *>(buf);

    for (std::size_t i = 0; i < size; ++i)
      new (&tbuf[i]) T{};

    return tbuf;
  }

private:
  std::vector<std::unique_ptr<char[]>> _bufs;
};

class HandlerCliRecv {
public:
  HandlerCliRecv() : _in(nullptr) {}

  void reset(SerialInBuff *in) { _in = in; }

  template <class T> void object_in(const T &) {}

  template <class T> void object_out(T &item) { *_in >> item; }

  template <class T> void buffer_in(T *, std::size_t) {}

  template <class T> void buffer_2d_in(T **&, std::size_t, std::size_t) {}

  template <class T, class Sizes>
  void buffer_2dvar_in(T **&, std::size_t, Sizes *) {}

  template <class T> void buffer_out(T *&ptr, std::size_t size) {
    for (std::size_t i = 0; i < size; ++i)
      *_in >> ptr[i];
  }

  template <class T>
  void buffer_2d_out(T **&ptr, std::size_t size1, std::size_t size2) {
    for (std::size_t i = 0; i < size1; ++i)
      for (std::size_t j = 0; j < size2; ++j)
        *_in >> ptr[i][j];
  }

  template <class T, class Sizes>
  void buffer_2dvar_out(T **&ptr, std::size_t size1, Sizes *sizes2) {
    for (std::size_t i = 0; i < size1; ++i)
      for (Sizes j = 0; j < sizes2[i]; ++j)
        *_in >> ptr[i][j];
  }

  void buffer_2d_in_cstr(char **&, std::size_t) {}

private:
  SerialInBuff *_in;
};

class HandlerCliSend {
public:
  HandlerCliSend() : _out(nullptr) {}

  void reset(SerialOutBuff *out) { _out = out; }

  template <class T> void object_in(const T &item) { *_out << item; }

  template <class T> void object_out(const T &) {}

  template <class T> void buffer_in(T *&ptr, std::size_t size) {
    for (std::size_t i = 0; i < size; ++i)
      *_out << ptr[i];
  }

  template <class T>
  void buffer_2d_in(T **&ptr, std::size_t size1, std::size_t size2) {
    for (std::size_t i = 0; i < size1; ++i)
      for (std::size_t j = 0; j < size2; ++j)
        *_out << ptr[i][j];
  }

  template <class T, class Sizes>
  void buffer_2dvar_in(T **&ptr, std::size_t size1, Sizes *sizes2) {
    for (std::size_t i = 0; i < size1; ++i)
      for (Sizes j = 0; j < sizes2[i]; ++j)
        *_out << ptr[i][j];
  }

  template <class T> void buffer_out(T *&, std::size_t) {}

  template <class T> void buffer_2d_out(T **&, std::size_t, std::size_t) {}

  template <class T, class Sizes>
  void buffer_2dvar_out(T **&, std::size_t, Sizes *) {}

  void buffer_2d_in_cstr(char **&ptr, std::size_t size) {
    // data is serialized as one big linear array of cstr, with \0 included
    // and the size of this linear array is written first (before array)

    // compute and write size
    std::size_t total_size = 0;
    for (std::size_t i = 0; i < size; ++i)
      total_size += std::strlen(ptr[i]) + 1;
    sb_serial_raw<std::uint16_t>(*_out, total_size);

    // Write all strs
    for (std::size_t i = 0; i < size; ++i)
      _out->write(ptr[i], std::strlen(ptr[i]) + 1);
  }

private:
  SerialOutBuff *_out;
};

class HandlerServRecv {
public:
  HandlerServRecv() : _in(nullptr) {}

  void reset(SerialInBuff *in) {
    _in = in;
    _tb.reset();
  }

  template <class T> void object_in(T &item) { *_in >> item; }

  template <class T> void object_out(T &) {}

  template <class T> void buffer_in(T *&ptr, std::size_t size) {
    auto tbuf = _tb.add_buff<T>(size);
    for (std::size_t i = 0; i < size; ++i)
      *_in >> tbuf[i];
    ptr = tbuf;
  }

  template <class T>
  void buffer_2d_in(T **&ptr, std::size_t size1, std::size_t size2) {
    // Create big buffer and store all content inside
    T *all_buf = _tb.add_buff<T>(size1 * size2);
    for (std::size_t i = 0; i < size1 * size2; ++i)
      *_in >> all_buf[i];

    // Create indirections buffer pointing to all_buf
    T **dir_buf = _tb.add_buff<T *>(size1);
    for (std::size_t i = 0; i < size1; ++i)
      dir_buf[i] = &all_buf[i * size2];

    ptr = dir_buf;
  }

  template <class T, class Sizes>
  void buffer_2dvar_in(T **&ptr, std::size_t size1, Sizes *sizes2) {
    // Compute total size
    Sizes all_size = 0;
    for (std::size_t i = 0; i < size1; ++i)
      all_size += sizes2[i];

    // Create big buffer and store all content inside
    T *all_buf = _tb.add_buff<T>(all_size);
    for (Sizes i = 0; i < all_size; ++i)
      *_in >> all_buf[i];

    // Create indirections buffer pointing to all_buf
    T **dir_buf = _tb.add_buff<T *>(size1);
    T *cur_ptr = all_buf;
    for (std::size_t i = 0; i < size1; ++i) {
      dir_buf[i] = cur_ptr;
      cur_ptr += sizes2[i];
    }

    ptr = dir_buf;
  }

  template <class T> void buffer_out(T *&ptr, std::size_t size) {
    ptr = _tb.add_buff<T>(size);
  }

  template <class T>
  void buffer_2d_out(T **&ptr, std::size_t size1, std::size_t size2) {
    // Alloc contiguous big buffer, and buffer of indirections
    T *all_buff = _tb.add_buff<T>(size1 * size2);
    T **dir_buff = _tb.add_buff<T *>(size1);

    // Make indirections point to all_buff
    for (std::size_t i = 0; i < size1; ++i)
      dir_buff[i] = &all_buff[i * size2];

    ptr = dir_buff;
  }

  template <class T, class Sizes>
  void buffer_2dvar_out(T **&ptr, std::size_t size1, Sizes *sizes2) {
    // Compute full size of 2d matrix
    Sizes total_size = 0;
    for (std::size_t i = 0; i < size1; ++i)
      total_size += sizes2[i];

    // Alloc contiguous big buffer, and buffer of indirections
    T *all_buff = _tb.add_buff<T>(total_size);
    T **dir_buff = _tb.add_buff<T *>(size1);

    // Make indirections point to all_buff
    T *cur_ptr = all_buff;
    for (std::size_t i = 0; i < size1; ++i) {
      dir_buff[i] = cur_ptr;
      cur_ptr += sizes2[i];
    }

    ptr = dir_buff;
  }

  void buffer_2d_in_cstr(char **&ptr, std::size_t size) {
    // Create and load full data buffer
    auto total_size = sb_unserial_raw<std::uint16_t>(*_in);
    char *full_buf = _tb.add_buff<char>(total_size);
    _in->read(full_buf, total_size);

    // Create and load indirection buffer
    char **dir_buf = _tb.add_buff<char *>(size);
    char *str = full_buf;
    for (std::size_t i = 0; i < size; ++i) {
      dir_buf[i] = str;
      auto str_size = std::strlen(str) + 1;
      str += str_size;
    }

    ptr = dir_buf;
  }

private:
  SerialInBuff *_in;
  TmpBuffHolder _tb;
};

class HandlerServSend {
public:
  HandlerServSend() : _out(nullptr) {}

  void reset(SerialOutBuff *out) { _out = out; }

  template <class T> void object_in(const T &) {}

  template <class T> void object_out(const T &item) { *_out << item; }

  template <class T> void buffer_in(T *&, std::size_t) {}

  template <class T> void buffer_2d_in(T **&, std::size_t, std::size_t) {}

  template <class T, class Sizes>
  void buffer_2dvar_in(T **&, std::size_t, Sizes *) {}

  template <class T> void buffer_out(T *&ptr, std::size_t size) {
    for (std::size_t i = 0; i < size; ++i)
      *_out << ptr[i];
  }

  template <class T>
  void buffer_2d_out(T **&ptr, std::size_t size1, std::size_t size2) {
    for (std::size_t i = 0; i < size1; ++i)
      for (std::size_t j = 0; j < size2; ++j)
        *_out << ptr[i][j];
  }

  template <class T, class Sizes>
  void buffer_2dvar_out(T **&ptr, std::size_t size1, Sizes *sizes2) {
    for (std::size_t i = 0; i < size1; ++i)
      for (Sizes j = 0; j < sizes2[i]; ++j)
        *_out << ptr[i][j];
  }

  void buffer_2d_in_cstr(char **&, std::size_t) {}

private:
  SerialOutBuff *_out;
};

#define HANDLER_DISPATCH1(Meth, V0)                                            \
  switch (_mode) {                                                             \
  case Mode::CLI_RECV:                                                         \
    _h_cli_recv.Meth(V0);                                                      \
    break;                                                                     \
  case Mode::CLI_SEND:                                                         \
    _h_cli_send.Meth(V0);                                                      \
    break;                                                                     \
  case Mode::SERV_RECV:                                                        \
    _h_serv_recv.Meth(V0);                                                     \
    break;                                                                     \
  case Mode::SERV_SEND:                                                        \
    _h_serv_send.Meth(V0);                                                     \
    break;                                                                     \
  };

#define HANDLER_DISPATCH2(Meth, V0, V1)                                        \
  switch (_mode) {                                                             \
  case Mode::CLI_RECV:                                                         \
    _h_cli_recv.Meth(V0, V1);                                                  \
    break;                                                                     \
  case Mode::CLI_SEND:                                                         \
    _h_cli_send.Meth(V0, V1);                                                  \
    break;                                                                     \
  case Mode::SERV_RECV:                                                        \
    _h_serv_recv.Meth(V0, V1);                                                 \
    break;                                                                     \
  case Mode::SERV_SEND:                                                        \
    _h_serv_send.Meth(V0, V1);                                                 \
    break;                                                                     \
  };

#define HANDLER_DISPATCH3(Meth, V0, V1, V2)                                    \
  switch (_mode) {                                                             \
  case Mode::CLI_RECV:                                                         \
    _h_cli_recv.Meth(V0, V1, V2);                                              \
    break;                                                                     \
  case Mode::CLI_SEND:                                                         \
    _h_cli_send.Meth(V0, V1, V2);                                              \
    break;                                                                     \
  case Mode::SERV_RECV:                                                        \
    _h_serv_recv.Meth(V0, V1, V2);                                             \
    break;                                                                     \
  case Mode::SERV_SEND:                                                        \
    _h_serv_send.Meth(V0, V1, V2);                                             \
    break;                                                                     \
  };

class RequestHandler;

template <class T> void prepare_request(RequestHandler &h, T &data);

class RequestHandler {

  enum class Mode {
    CLI_RECV,
    CLI_SEND,
    SERV_RECV,
    SERV_SEND,
  };

public:
  RequestHandler(bool is_server) : _server(is_server) {}

  /// Write client data request
  template <class T> void client_write_request(SerialOutBuff &out, T &data) {
    assert(!_server);
    _mode = Mode::CLI_SEND;
    _h_cli_send.reset(&out);
    prepare_request(*this, data);
  }

  /// Read client data response
  template <class T> void client_read_response(SerialInBuff &in, T &data) {
    assert(!_server);
    _mode = Mode::CLI_RECV;
    _h_cli_recv.reset(&in);
    prepare_request(*this, data);
  }

  template <class T> void server_read_request(SerialInBuff &in, T &data) {
    assert(_server);
    _mode = Mode::SERV_RECV;
    _h_serv_recv.reset(&in);
    prepare_request(*this, data);
  }

  template <class T> void server_write_response(SerialOutBuff &out, T &data) {
    assert(_server);
    _mode = Mode::SERV_SEND;
    _h_serv_send.reset(&out);
    prepare_request(*this, data);
  }

  /// Add a data object that belong to the input request
  /// Can be any kind, as long as it can be serial / unserial
  template <class T> void object_in(T &item) {
    HANDLER_DISPATCH1(object_in, item);
  }

  /// Add a data object that belong to the output request
  /// Can be any kind, as long as it can be serial / unserial
  template <class T> void object_out(T &item) {
    HANDLER_DISPATCH1(object_out, item);
  }

  /// Add a buffer of size items of type T
  /// When request sent, the buffer content is serialized
  /// When the request is received, the buffer is unserialized into some
  /// temporary memory
  /// And `ptr` is set to point to this memory
  /// The memory is valid until the response is sent
  template <class T> void buffer_in(T *&ptr, std::size_t size) {
    HANDLER_DISPATCH2(buffer_in, ptr, size);
  }

  /// Add a buffer T[size1][size2], with indirection pointer
  /// When request sent, the whole 2d content is serialized
  /// When the request is received, the buffer is unserialized into some
  /// temporary memory, and a temporary indirection buffer with right pointer
  /// is set And `ptr` is set to point to this temporary indirection buffer
  /// The memory is valid until the response is sent
  template <class T>
  void buffer_2d_in(T **&ptr, std::size_t size1, std::size_t size2) {
    HANDLER_DISPATCH3(buffer_2d_in, ptr, size1, size2);
  }

  /// Similar to buffer_2d_in, but second dimension varies
  /// the size of each T[i] is sizes2[i]
  template <class T, class Sizes>
  void buffer_2dvar_in(T **&ptr, std::size_t size1, Sizes *sizes2) {
    HANDLER_DISPATCH3(buffer_2dvar_in, ptr, size1, sizes2);
  }

  /// Add a buffer of size items of type T
  /// When request is received, a buffer is allocated to hold these items
  /// When response send, this buffer content is serialized
  /// When response received, content unserialized into `ptr`
  template <class T> void buffer_out(T *&ptr, std::size_t size) {
    HANDLER_DISPATCH2(buffer_out, ptr, size);
  }

  /// Add a buffer T[size1][size2], with indirection pointer
  /// When request is received, a buffer is allocated to hold these items
  /// When response send, this buffer content is serialized
  /// When response received, content unserialized into `ptr`
  template <class T>
  void buffer_2d_out(T **&ptr, std::size_t size1, std::size_t size2) {
    HANDLER_DISPATCH3(buffer_2d_out, ptr, size1, size2);
  }

  /// Similar to buffer_2d_out, but second dimension varies
  /// the size of each T[i] is sizes2[i]
  template <class T, class Sizes>
  void buffer_2dvar_out(T **&ptr, std::size_t size1, Sizes *sizes2) {
    HANDLER_DISPATCH3(buffer_2dvar_out, ptr, size1, sizes2);
  }

  /// Like buffer_2d_in, but this a special version to handle zero-terminated
  /// strings of varying size
  void buffer_2d_in_cstr(char **&ptr, std::size_t size) {
    HANDLER_DISPATCH2(buffer_2d_in_cstr, ptr, size);
  }

private:
  bool _server;
  HandlerCliRecv _h_cli_recv;
  HandlerCliSend _h_cli_send;
  HandlerServRecv _h_serv_recv;
  HandlerServSend _h_serv_send;
  Mode _mode;
};

} // namespace odb
