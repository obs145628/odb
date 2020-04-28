//===-- server/cli-client-handler.hh - CLIClientHandler class ---*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of ClientHandler that doesn't communicate with a client
/// process, but instead read commands directly from stdin.
///
//===----------------------------------------------------------------------===//

#include "client-handler.hh"

#include "../mess/db-client.hh"
#include "../mess/simple-cli-client.hh"

namespace odb {

/// Doesn't communicate with any other client processes
/// Read commands from stdin, line by line, and write output to stdout
/// Commands may he handled by one or many CLI utils
/// For now, only `SimpleCLIClient` is supported.
class CLIClientHandler : public ClientHandler {

public:
  CLIClientHandler(Debugger &db, const ServerConfig &conf);
  ~CLIClientHandler() override;

  void setup_connection() override;

  void run_command() override;

  void check_stopped() override;

private:
  DBClient _db_client;
  SimpleCLIClient _client;
  bool _is_tty;

  bool _catch_sigint;

  void _on_disconnect();
};

} // namespace odb
