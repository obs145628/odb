//===-- server/debugger.hh - Debugger class definition ----------*- C++ -*-===//
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

#include "../utils/range-map.hh"
#include "fwd.hh"
#include "vm-api.hh"
#include <map>
#include <memory>
#include <set>
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
  enum class State {
    NOT_STARTED,       // Haven't executed the first ins yet
    STOPPED,           // Stopped at a specific code position
    RUNNING_TOFINISH,  // Ignore breakpoints
    RUNNING_BKP,       // Stop at next breakpoints
    RUNNING_STEP,      // Stop at next instruction
    RUNNING_STEP_OVER, // Stop at next instruction (or doing it all if it's a
                       // call), or at a breakpoint
    RUNNING_STEP_OUT,  // Stop when returning from current subroutine, or at a
                       // breakpoint
    ERROR,             // program termidated because of an error
    EXIT,              // program termidated because of a normal exit
  };

  /// Create a debugger connected to a specific vm through a VMAPI instance
  /// After initialization, start in RUNNING_TOFINISH mode
  Debugger(std::unique_ptr<VMApi> &&vm);

  Debugger(const Debugger &) = delete;

  /// This function must be called right before the execution of the first
  /// instruction, when the VM is completely setup and ready to start execution
  /// It get all the general VM infos (nb regs, memory size, etc), and the
  /// program entry point.
  /// Set the Debugger to 'RUNNING_TOFINISH' state
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
  vm_reg_t find_reg_id(const std::string &name);

  /// Total number of registers available
  /// This may be a huge number for machines with infinite (or almost) number of
  /// registers
  vm_reg_t registers_count();

  /// List all registers of a specific kind
  /// For infinite or very high number of general purpose registers, they aren't
  /// listed
  /// Returns a reference to a private member
  /// Remains valid for the whole Debugger lifetime
  const std::vector<vm_reg_t> &list_regs(RegKind kind) const;

  /// Read `size` bytes of data from `addr` and store in `out_buf`
  void read_mem(vm_ptr_t addr, vm_size_t size, std::uint8_t *out_buf);

  /// Write `size` bytes of data to `addr` from `buf` src
  void write_mem(vm_ptr_t addr, vm_size_t size, const std::uint8_t *buf);

  /// Get full memory size
  vm_size_t get_memory_size();

  /// @returns symbol id of symbol def located at `addr`
  /// Returns VM_SYM_NULL if no symbol found
  vm_sym_t get_symbol_at(vm_ptr_t addr);

  /// Returns all symbols defined in [`addr`, `addr` + `size`[
  std::vector<vm_sym_t> get_symbols(vm_ptr_t addr, vm_size_t size);

  /// Returns infos about a symbol given it's index `idx`
  SymbolInfos get_symbol_infos(vm_sym_t idx);

  /// Returns the total number of symbols
  /// Can be used to know if it's now too expensive to get all symbols at once
  vm_sym_t symbols_count();

  /// Returns a symbol id given its name
  vm_sym_t find_sym_id(const std::string &name);

  /// Get the text format of the instruction or data directive at address `addr`
  /// Store in `addr_dist` the number of opcode bytes read
  /// This may be used to read instruction, but also data directives (eg .byte,
  /// .word)
  /// If the code contains a reference to a symbol, it's written '{symbol_idx}'
  /// Returns an empty string if there is no code or the opcode is invalid
  std::string get_code_text(vm_ptr_t addr, vm_size_t &addr_dist);

  /// Returns the address of the next instruction to be executed
  /// This may differ from PC on some VMs that have different fetch, decode,
  /// exec cycles
  vm_ptr_t get_execution_point();

  /// Returns the size in bytes of a pointer in the VM
  vm_size_t pointer_size();

  /// Returns the size in bytes of an int in the VM
  vm_size_t integer_size();

  /// Returns true if the VM instructions use a binary opcode
  bool use_opcode();

  /// Put a breakpoint at `addr`, no matter if this address is reachable
  /// Throws an error if there is already a breakpoint
  /// Or if `addr` outside of memory space
  void add_breakpoint(vm_ptr_t addr);

  /// Returns true if there is a breakpoint at `addr`
  /// Throws an error if `adddr` outside of memory space
  bool has_breakpoint(vm_ptr_t addr);

  /// Delete the breakpoint at `addr` if there is one
  /// Throws an error if `adddr` outside of memory space or if there is no
  /// breakpoint
  void del_breakpoint(vm_ptr_t addr);

  /// Resume program execution
  /// Throws if program finished
  void resume(ResumeType type);

  /// Switch the debugger to STOPPED state at the actual point in execution
  /// Throws if program finished or already stopped
  void stop();

  State get_state() const { return _state; }

  /// Return static informations about the VM
  /// Doesn't change during execution
  const VMInfos &get_vm_infos() const { return _infos; }

  // Current Call stack
  // The last element is about the function currently running, and call_addr is
  // garbage data
  const CallStack &get_call_stack() const { return _call_stack; }

private:
  std::unique_ptr<VMApi> _vm;
  VMInfos _infos;
  State _state;
  CallStack _call_stack;
  vm_ptr_t _ins_addr;

  std::map<vm_reg_t, RegInfos> _map_regs;
  std::map<std::string, vm_reg_t> _smap_regs;

  // @TIP Implem based on the fact that all symbols have different names
  // May not be always true for all VMs ?
  std::map<vm_sym_t, SymbolInfos> _map_syms;
  std::map<std::string, vm_sym_t> _smap_syms;
  std::unique_ptr<RangeMap<int>> _syms_ranges;
  std::map<vm_ptr_t, vm_sym_t> _syms_pos;

  std::set<vm_ptr_t> _breakpts;
  std::size_t
      _step_over_depth; // to be able to stop a the right subroutine return

  // Load all informations concerning a register in the data members
  // Does nothing if already loaded
  void _load_reg(vm_reg_t id);

  // Make sure all symbols in [addr, addr + size[ are in _map_syms
  // Use a range-based map to known which part of the memory need to be loaded
  void _preload_symbols(vm_ptr_t addr, vm_size_t size);

  // Call preload_symbols, but with `addr` in the middle range
  void _preload_symbols(vm_ptr_t addr);

  /// Load all informations concerning a symbol in the data members
  /// Does nothing is already loaded
  void _load_symbol(vm_sym_t id);
};

} // namespace odb
