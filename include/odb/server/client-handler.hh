//===-- server/client-handler.hh - ClientHandler definition -----*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Abstract class that bridge communcation between any kind of Client and the
/// Debugger API
///
//===----------------------------------------------------------------------===//

#pragma once

#include "fwd.hh"
#include <exception>
#include <string>
#include <vector>

namespace odb {

/// The ClientHandler is an abstract class
/// Every Child class comunicates with a different kind of client.
/// It receive commands from the client, call the right API functions of the
/// Debugger, and give the results to the client
/// It waits for the connection of one client, and stop working as soon as the
/// client is disconnected (there is no reconnection system yet)
/// It cannot handle more than one client
/// When the client get disconnected, the debug session is aborted and the
/// program run until completion
class ClientHandler {

public:
  enum class State {
    NOT_CONNECTED, // Waiting for a connection
    CONNECTED,     // Talking with client
    DISCONNECTED,  // end of connection
  };

  ClientHandler(Debugger &debugger, const ServerConfig &conf);
  ClientHandler(const ClientHandler &) = delete;
  virtual ~ClientHandler() = default;

  State get_state() const { return _state; }

  /// The first time this function is called, the impl start to wait for
  /// connections The call must not be blocking Instead, the function is called
  /// at every iteration of the DebuggerLoop as long as there is no connected
  /// client, in order to check if a client was connected.
  virtual void setup_connection() = 0;

  /// This function must read and exec one/many Debugger command.
  /// Or returns without doing anything if the client get disconnected.
  /// This function is called on the Debugger loop only when the Debugger is in
  /// stopped state.
  /// This function should block until it gets a command from a client, or the
  /// client gets deconneted.
  virtual void run_command() = 0;

  /// This function is called when the VM is running, to check if received a
  /// stop command, and execute it. Contrary to run_command, this function must
  /// not block to read commands
  virtual void check_stopped() = 0;

protected:
  /// Called by Child class when a client get connected
  void _client_connected();

  /// Called by parent Class when a client get disconnected
  void _client_disconnected();

  Debugger &get_debugger() { return _debugger; }

  const ServerConfig &get_conf() { return _conf; }

private:
  Debugger &_debugger;
  const ServerConfig &_conf;
  State _state;
};

} // namespace odb
