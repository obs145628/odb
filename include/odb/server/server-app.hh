//===-- server/server-app.hh - ServerApp class definition -------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// The master class server side, responsible for controlling all the parts of
/// the debugger: core db, program loop, communication with clients
///
//===----------------------------------------------------------------------===//

#pragma once

#include <functional>
#include <memory>

#include "client-handler.hh"
#include "debugger.hh"
#include "fwd.hh"

namespace odb {

struct ServerConfig {
  // If true, the debugger is running
  // default is false
  // env: ODB_CONF_ENABLED=0/1
  bool enabled;

  // If true, the execution is stopped before the first instruction
  // Used to be able to debug a program from the beginning
  // default is false
  // env: ODB_CONF_NOSTART=0/1
  bool nostart;

  // If true, the CLI on Server mode is enabled
  // With this mode, the debugguer is controled direcly from the VM process,
  // with a CLI reading commands from stdin
  // When enabled, the VM is stopped at the beginning and the CLI appears right
  // away.
  // default is false
  // env: ODB_CONF_MODE_SERVER_CLI=0/1
  bool mode_server_cli;

  // If true and ServerCLI is used, a SIGINT signal handler is set that will
  // stop the VM execution This way, it's possible to stop execution and get
  // back the command line using Ctrl-C
  // default is true
  // env: ODB_CONF_SERVER_CLI_SIGHANDLER
  bool server_cli_sighandler;

  // If true, a TCP server is run.
  // Whis this mode, can connect to the Debugger from another process with TCP
  // client default is false env: ODB_CONF_MODE_TCP=0/1
  bool mode_tcp;

  // The port the server listens to in TCP_PORT mode
  // default is 12644
  // env: ODB_CONF_TCP_PORT=<int>
  int tcp_port;
};

/// To setup a DB Server, an instance of this class must be created
/// Main class, responsible for controlling every part of ODB server-side
///
/// It has many options, and can be configured by 2 ways:
/// - with a config object given to the constructor
/// - using the default constructor object (recommended)
///   The default disable everything, the debugger is turned OFF
/// - with env variables that can override config values
class ServerApp {
public:
  using api_builder_f = std::function<std::unique_ptr<VMApi>()>;

  /// `api_builder` is a functor called when needed to build the debugger core
  /// It is never called if the debugger isn't enabled
  ServerApp(const ServerConfig &conf, const api_builder_f &api_builder);

  /// Instantiate with default config
  ServerApp(const api_builder_f &api_builder);

  ServerApp(const ServerApp &) = delete;

  /// Enter the debugger loop
  /// Must be called right before executing each instruction
  /// More infos in `docs/design.txt`
  void loop();

private:
  ServerConfig _conf;
  api_builder_f _api_builder;
  std::unique_ptr<Debugger> _db;
  std::unique_ptr<ClientHandler> _client;

  // init debugger
  void _init();

  // disable debugger and clear all allocated memory
  void _shutdown();

  // block until `_client` connected
  void _connect();

  // returns true if db stopped (breakpoint, or exit/crash)
  bool _db_is_stopped();

  // Stop debugger if not stopped already
  void _stop_db();
};

} // namespace odb
