//===-- client/tcp-data-client.hh - TcpDataClient class ---------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// TcpDataClient class definition
///
//===----------------------------------------------------------------------===//

#pragma once

#include "abstract-data-client.hh"

#include <string>

namespace odb {

/// Send debugguer commands using TCP protocol
/// More infos about protocol in mess/tcp-transfer.hh
class TCPDataClient : public AbstractDataClient {

public:
  TCPDataClient(const std::string &hostname, int port);

  ~TCPDataClient() override;

  bool connect() override;

  bool send_data(const SerialOutBuff &os) override;

  bool recv_data(SerialInBuff &is) override;

private:
  std::string _hostname;
  int _port;
  int _fd;
};

} // namespace odb
