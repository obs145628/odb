//===-- mess/simple-cli-client.hh - SimpleCLIClient class -------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// SimpleCLIClient class definition. Can be used on client or server side
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../server/fwd.hh"
#include "fwd.hh"
#include <string>
#include <vector>

namespace odb {

/// This is a basic CLI to communicate with the Debugger
/// Can be used either Server Side or client side
/// It takes a string (line command), parse it, use DBClient API, and produce a
/// new string with the results.
///
/// Available commands:
/// Each command is a space-separated list of arguments
///
///
/// Print registers:
/// preg <type> <reg>+
///
///
/// Update registers
/// sreg <type> (<reg> <val>)+
///
/// Print registers infos
/// pregi <reg>+
///
/// Read memory
/// pmem <type> <val> (addr) <int> (nb items)
///
/// Write memory
/// smem <type> <val> (addr) <val>+
///
/// Print symbol informations
/// psym <symbol>
///
/// Print code at current position
/// code <int=3> (nb-lines, before and after current position)
///
/// Add breakpoint
/// b <val> (address)
///
/// Delete breakpoint
/// delb <val> (address)
///
/// Resume execution:
/// c / continue (continue)
/// s / step (step)
/// n / next (step over)
/// fin / finish (step out)
///
/// Print current state informations (addr, stack)
/// state
///
/// Print call stack
/// bt
///
/// Print VM informations
/// vm
///
///
/// Usual arguments:
///   <type>: u8, i8, u16, i16, u32, i32, u64, i64, f32, f64
///    <reg>: '%' (<reg-name> | <reg-id>)
///    <val>: <int> | <float> | <symbol>
///            (If symbol, val is symbol address)
/// <symbol>: '@' (<symbol-name> | <symbol-id>)
/// <int> : signed in base 2/8/10/16
///   prefix 0b, 0, and 0x to change bases
///

class SimpleCLIClient {
public:
  SimpleCLIClient(DBClient &env);
  SimpleCLIClient(const SimpleCLIClient &) = delete;

  std::string exec(const std::string &cmd);

private:
  DBClient &_env;
  std::vector<std::string> _cmd;

  std::string _cmd_preg();

  std::string _cmd_sreg();

  std::string _cmd_pregi();

  std::string _cmd_pmem();

  std::string _cmd_smem();

  std::string _cmd_psym();

  std::string _cmd_code();

  std::string _cmd_b();

  std::string _cmd_delb();

  std::string _cmd_continue();

  std::string _cmd_step();

  std::string _cmd_next();

  std::string _cmd_finish();

  std::string _cmd_state();

  std::string _cmd_bt();

  std::string _cmd_vm();
};

} // namespace odb
