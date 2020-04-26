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

  template <class T> void buffer_out(T *&ptr, std::size_t size) {
    for (std::size_t i = 0; i < size; ++i)
      *_in >> ptr[i];
  }

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

  template <class T> void buffer_out(T *&, std::size_t) {}

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

  template <class T> void buffer_out(T *&ptr, std::size_t size) {
    ptr = _tb.add_buff<T>(size);
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

  template <class T> void buffer_out(T *&ptr, std::size_t size) {
    for (std::size_t i = 0; i < size; ++i)
      *_out << ptr[i];
  }

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

  /// Add a buffer of size items of type T
  /// When request is received, a buffer is allocated to hold these items
  /// When response send, this buffer content is serialized
  /// When response received, content unserialized into `ptr`
  template <class T> void buffer_out(T *&ptr, std::size_t size) {
    HANDLER_DISPATCH2(buffer_out, ptr, size);
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
