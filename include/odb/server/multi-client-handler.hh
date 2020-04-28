//===-- server/multi-client-handler.hh - MultiClientHandler  ----*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of ClientHandler that can handle multiple client types
///
//===----------------------------------------------------------------------===//

#include "client-handler.hh"

#include <vector>

#include "../mess/db-client.hh"
#include "../mess/simple-cli-client.hh"

namespace odb {

/// Implementation to handle multiple type of client handlers
/// Create a list of client handlers and try to connect to a client with all
/// As soon as one the handler establish a connection, it becomes the main one,
/// and all others are dropped
class MultiClientHandler : public ClientHandler {

public:
  MultiClientHandler(Debugger &db, const ServerConfig &conf);

  /// Shortcut for ServerApp to avoid waiting for clients if they are no
  /// handlers
  bool empty() const { return _wait.empty() && _main.get() == nullptr; };

  void setup_connection() override;

  void run_command() override;

  void check_stopped() override;

private:
  std::vector<std::unique_ptr<ClientHandler>> _wait;
  std::unique_ptr<ClientHandler> _main;
};

} // namespace odb
