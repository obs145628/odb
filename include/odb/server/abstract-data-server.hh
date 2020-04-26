//===-- server/abstract-data-server.hh - AbstractDataServer -----*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// AbstractDataServer class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../mess/fwd.hh"

namespace odb {

/// Abstract class that represents a server that can send and recv binary data
/// packets All calls are blocking.
/// This is used to receive and give commands to the core debugger
/// An extra layer is needed on top of this because server communications cannot
/// block main thread
///
/// This system can only connect with one client
class AbstractDataServer {

public:
  AbstractDataServer() = default;
  virtual ~AbstractDataServer() = default;

  /// Returns true if connection was successfull
  virtual bool connect() = 0;

  /// Returns true if the whole buff could be sent
  virtual bool send_data(const SerialOutBuff &os) = 0;

  /// Returns true if the whole buff could be read
  /// Store read data in `is`
  virtual bool recv_data(SerialInBuff &is) = 0;
};

} // namespace odb
