//===-- server/debugger.hh - Debugger class definition ---------*- C++//-*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Request struct represent data sent by client to the server.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "fwd.hh"
#include <exception>
#include <string>
#include <vector>

namespace odb {

/// The Debugger is where all the debugging logic happens
/// This is a generic debugger, supposed to work for many different registers
/// It handles request of many kinds:
/// - related to registers. It can be a basic small number of registers, or also
///    it can handle VM with very high / infinite number of registers
/// - memory: get/set into memory
/// - symbols and code (instructions and data directives)
/// - breakpoints
/// - program execution
///
/// The debugger has a call frames, an can be step in / out / over subroutines
/// This is done using the callback interface that has a method to know if the
/// last executed instruction was a call, a ret, or none of the 2.
class Debugger {

public:
  class Error : public std::exception {

  public:
    Error(std::string msg) : _msg(msg) {}

    const char *what() const throw() { return _msg.c_str(); }

  private:
    std::string _msg;
  };

  enum class State {
    NOT_STARTED, // Haven't executed the first ins yet
    STOPPED,     // Stopped at a specific code position
    RUNNING,
    TERMIDATED, // either normal exit or crash
  };

  struct CallInfos {
    vm_ptr_t
        caller_start_addr; // Address of the subroutine where the call is made
    vm_sym_t
        caller_sym; // Symbol of the caller subroutine, or -1 if doesn't have
    vm_ptr_t call_addr; // Address of the instruction that made the call
  };

  using CallStack = std::vector<CallInfos>;

  /// This function must be called right before the execution of the first
  /// instruction, when the VM is completely setup and ready to start execution
  /// It get all the general VM infos (nb regs, memory size, etc), and the
  /// program entry point.
  void on_init();

  /// This function must be called right after the execution of every
  /// instruction by the VM.
  /// Making an API call between an instruction execution and the call to
  /// `on_update` leads to incorrect results.
  /// It gets some infos about the VM to keep track of the program flow
  void on_update();

  // ##### Debugger API #####
  // All functions may have errors
  // (Bad client request from user)
  // A Debugger::Error is thrown when it happens

  /// Read the content of register with index `idx`
  /// Value will be written in `val`
  /// Must be big enough to hold the value
  void get_reg(vm_reg_t idx, std::uint8_t *val);

  /// Change the content of register with index `idx`
  /// Value is get from buffer `new_val`
  /// Must be big enough
  void set_reg(vm_reg_t idx, const std::uint8_t *new_val);

  /// Get informations about a specific register
  RegInfos get_reg_infos(vm_reg_t idx);

  /// Returns a register id given its name
  vm_reg_t find_reg_id(const char *name);

  /// Total number of registers available
  /// This may be a huge number for machines with infinite (or almost) number of
  /// registers
  vm_reg_t registers_count();

  /// List all registers of a specific kind
  /// For infinite or very high number of general purpose registers, they aren't
  /// listed
  std::vector<vm_reg_t> list_regs(RegKind kind);

  /// Read `size` bytes of data from `addr` and store in `out_buf`
  void read_mem(vm_ptr_t addr, vm_size_t size, std::uint8_t *out_buf);

  /// Write `size` bytes of data to `addr` from `buf` src
  void write_mem(vm_ptr_t addr, vm_size_t size, const std::uint8_t *buf);

  /// Get full memory size
  vm_size_t get_memory_size();

  /// Returns all symbols defined in [`addr`, `addr` + `size`[
  std::vector<vm_sym_t> get_symbols(vm_ptr_t addr, vm_size_t size);

  /// Returns infos about a symbol given it's index `idx`
  SymbolInfos get_symbol_infos(vm_sym_t idx);

  /// Returns the total number of symbols
  /// Can be used to know if it's now too expensive to get all symbols at once
  vm_sym_t symbols_count();

  /// Returns a symbol id given its name
  vm_sym_t find_sym_id(const char *name);

  /// Get the text format of the instruction or data directive at address `addr`
  /// Store in `addr_dist` the number of opcode bytes read
  /// This may be used to read instruction, but also data directives (eg .byte,
  /// .word)
  /// If the code contains a reference to a symbol, it's written '{symbol_idx}'
  std::string get_code_text(vm_ptr_t addr, vm_size_t &addr_dist);

  /// Put a breakpoint at `addr`, no matter if this address is reachable
  /// Throws an error if there is already a breakpoint
  /// Or if `addr` outside of memory space
  void add_breakpoint(vm_ptr_t addr);

  /// Returns true if there is a breakpoint at `addr`
  /// Throws an error if `adddr` outside of memory space
  bool has_breakpoint(vm_ptr_t addr);

  /// Delete the breakpoint at `addr` if there is one
  void del_breadkpoint(vm_ptr_t addr);

  /// Resume program execution
  void resume(ResumeType type);

  State get_state() const { return _state; }

  // Current Call stack
  // The last element is about the function currently running, and call_addr is
  // garbage data
  const CallStack &get_call_stack() const { return _call_stack; }

private:
  State _state;
  CallStack _call_stack;
};

} // namespace odb
