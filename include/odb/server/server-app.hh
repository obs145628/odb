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

  // If true and ServerCLI is used, a SIGINT signal handler is set that will
  // stop the VM execution This way, it's possible to stop execution and get
  // back the command line using Ctrl-C
  // default is true
  // env: ODB_CONF_SERVER_CLI_SIGHANDLER
  bool server_cli_sighandler;
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

  // Global variable used to stop the program
  // Check inside `loop()`, if set to true, _db->stop() is called
  // This is a global to be able to stop the DB on signal handlers
  static bool g_force_stop_db;

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

  // block until `_client` connected
  void _connect();

  // returns true if db stopped (breakpoint, or exit/crash)
  bool _db_is_stopped();

  // Stop debugger if not stopped already
  void _stop_db();
};

} // namespace odb
