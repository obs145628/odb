//===-- client/abstract-data-client.hh - AbstractDataClient -----*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// AbstractDataClient class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../mess/fwd.hh"

namespace odb {

/// Abstract class that represents a client that can send and recv binary data
/// packets All calls are blocking This is used to send commands to the core
/// debugguer running on the VM process. Commands are serialized and sent with
/// this interface
class AbstractDataClient {

public:
  AbstractDataClient() = default;
  virtual ~AbstractDataClient() = default;

  /// Returns true if connection was successfull
  virtual bool connect() = 0;

  /// Returns true if the whole buff could be sent
  virtual bool send_data(const SerialOutBuff &os) = 0;

  /// Returns true if the whole buff could be read
  /// Store read data in `is`
  virtual bool recv_data(SerialInBuff &is) = 0;
};

} // namespace odb
