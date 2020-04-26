//===-- mess/tcp-transfer.hh - TCP utils ------------------------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Serial util functions to send/recv SerialBuff objects through TCP sockets
///
//===----------------------------------------------------------------------===//

#pragma once

#include "fwd.hh"

namespace odb {

// Communication protocol:
//
// Send SerialOutBuff: header + SerialOutBuff binary array content
//   header: binary array size in bytes, as std::uint32_t
//
// Recv SerialInbuff: header + SerialInbuff binary array content
//   header: binary array size in bytes, as std::uint32_t

/// Write `os` content to file descriptor `fd`
/// Blocking call
/// @returns true if write everything successfully
bool send_data_tcp(const SerialOutBuff &os, int fd);

// Read `is` content from file descriptor `fd`
/// Blocking call
/// @returns true if write everything successfully
bool recv_data_tcp(SerialInBuff &is, int fd);

} // namespace odb
