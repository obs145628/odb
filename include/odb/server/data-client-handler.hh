//===-- server/data-client-handler.hh - DataClientHandler class -*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of ClientHandler that gets commands from a DataClient
///
//===----------------------------------------------------------------------===//

#include "client-handler.hh"

#include <memory>
#include <thread>

#include "fwd.hh"

namespace odb {

// Internal class to run blocking server on another thread
class DataClientServerRunner;

/// Read serialized commands from a specific kind of DataClient
/// Use another thread internally to avoid blocking VM.
class DataClientHandler : public ClientHandler {

public:
  enum class Kind {
    TCP_SERVER,
  };

  DataClientHandler(Debugger &db, const ServerConfig &conf, Kind kind);
  ~DataClientHandler() override;

  void setup_connection() override;

  void run_command() override;

  void check_stopped() override;

private:
  Kind _kind;
  std::unique_ptr<DataClientServerRunner> _runner;

  void _init();
};

} // namespace odb
