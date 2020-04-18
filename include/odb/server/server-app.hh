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

#include "fwd.hh"

namespace odb {

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
  struct Config {
    // If true, the debugger is running
    // default is false
    // env: ODB_CONF_ENABLED=0/1
    bool enabled;

    // If true, the execution is stopped before the first instruction
    // Used to be able to debug a program from the beginning
    // default is false
    // env: ODB_CONF_NOSTART=0/1
    bool nostart;
  };

private:
};

} // namespace odb
