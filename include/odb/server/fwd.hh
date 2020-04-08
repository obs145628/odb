//===-- server/fwd.hh - Forward definitions --------------------*- C++//-*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Foward class defs, and aliases for many int types used in debug environment,
/// to have more meaningful names
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace odb {

using vm_reg_t = std::uint32_t; // register index
using vm_ptr_t = std::uint64_t; // memory address
using vm_size_t = std::uint64_t;
using vm_ssize_t = std::int64_t;
using vm_sym_t = std::uint32_t; // symbol index

enum class RegKind {
  general,
  program_counter,
  stack_pointer,
  base_pointer,
  flags,
};

enum class ResumeType {
  ToFinish, // run program until exit or crash, ignoring all breakpoints

  Continue, // run until next breakpoint

  Step, // run the next instruction

  StepOver, // run next instruction, if it's a call, keep running until
            // returning from the subroutine, or stop before on breakpoints

  StepOut, // run instructions until returning from current subroutine, or stop
           // before on breakpoints
};

struct RegInfos {
  vm_reg_t idx;
  std::string name;
  vm_size_t size;
  RegKind kind;
  std::vector<std::uint8_t> val;
};

struct SymbolInfos {
  vm_sym_t idx;
  std::string name;
  vm_ptr_t addr;
};

class Debugger;

constexpr vm_sym_t SYM_ID_NONE = static_cast<vm_sym_t>(-1);

} // namespace odb
