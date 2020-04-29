//===-- apps/odb-cli/cli.hh - CLI class definition --------------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Centralize client/server interface to communicate with debugger
///
//===----------------------------------------------------------------------===//

#pragma once

#include "odb/client/db-client-impl-data.hh"
#include "odb/mess/db-client.hh"
#include "odb/mess/simple-cli-client.hh"

class CLI {
public:
  CLI(int argc, char **argv);

  // Is state switched from VM stopped to running ?
  // If true, usually needs to display infos
  bool state_switched();

  /// Wait until ready for next command
  /// Returns false if deconneted
  bool next();

  /// Exec command with SimpleCLIClient
  std::string exec(const std::string &cmd);

private:
  odb::DBClient _db_client;
  odb::SimpleCLIClient _cli;
  bool _state_switch;

  void _setup();

  void _wait_stop();
};
