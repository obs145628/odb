//===-- server/tcp-data-server.hh - TcpDataServer class ---------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// TcpDataServer class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include "abstract-data-server.hh"

#include <string>

namespace odb {

/// Send debugguer commands using TCP protocol
/// More infos about protocol in mess/tcp-transfer.hh
class TCPDataServer : public AbstractDataServer {

public:
  TCPDataServer(int port);

  ~TCPDataServer() override;

  bool connect() override;

  bool send_data(const SerialOutBuff &os) override;

  bool recv_data(SerialInBuff &is) override;

private:
  int _port;
  int _serv_fd;
  int _cli_fd;
};

} // namespace odb
