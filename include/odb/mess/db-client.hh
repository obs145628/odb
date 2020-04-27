//===-- mess/db-client.hh - DBClient class definition -----------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// The main class to communicate with a a debugguer
///
//===----------------------------------------------------------------------===//

#pragma once

#include <map>
#include <memory>
#include <vector>

#include "../server/fwd.hh"
#include "fwd.hh"

namespace odb {

/// Internal data structure received after each block
struct DBClientUpdate {
  StoppedState vm_state;
  bool stopped;
  vm_ptr_t addr; // execution point
  CallStack stack;
};

/// This is the main class used to communicate with a running debugguer
/// It's most often used on the client process, and takes care of serializing
/// and sending commands to debugguer
///
/// Can also be used directly on the same process as the debugguer, but a little
/// differently In that situation, no serialization is done, and right calls are
/// simply made
///
///
///
/// @EXTRA for now only redirect calls, but caching to limit requests could help
class DBClient {
public:
  /// The DB client can be on 4 states:
  /// - not-connected: cannot connected yet. can only call connect() method
  /// - discconnected: connection lost, cannot do anything with the object
  /// - VMStopped: main state where the object is usable. can do everything but
  /// call connect(), stop() and check_stopped()
  /// - VMRunning: Can anly call stop() and check_stopped()
  enum class State {
    NOT_CONNECTED,
    DISCONNECTED,
    VM_STOPPED,
    VM_RUNNING,
  };

  DBClient(std::unique_ptr<DBClientImpl> &&impl);

  State state() const { return _state; }

  /// Blocking calls
  /// All these calls may need to interact with the server
  /// If the request is invalid and Debugger throws, these methods rethrow the
  /// same exceptions

  /// Can only be called in NOT_CONNECTED state
  /// Throws if connection failed
  /// If succeeds, state move to VM_STOPPED or VM_RUNNING
  void connect();

  /// Can only be called in VM_RUNNING state
  /// Send a stop command to the debugger
  /// Throws if the stop failed
  void stop();

  /// Can only be called in VM_RUNNING state
  /// Send a request to know if the debugguer program is still running.
  /// This method also update state accordingly
  /// The server never sends a message when the VM stopped
  /// So instead the client must ask in a loop if the VM is still running
  void check_stopped();

  /// All following functions can only be called in VM_STOPPED state

  /// Load many registers at once
  /// @param ids array of register ids
  /// @param out_bufs array of buffer where every register value will be written
  /// @param regs_size array of the size of each register. If all registers have
  /// the same size, you can write 0 in regs_size[1]
  /// @param nregs size of arrays
  ///
  /// If async, once completed the output arguments are not filled
  /// The method must be called again
  /// If any output is NULL, only fill the cache.
  void get_regs(const vm_reg_t *ids, char **out_bufs,
                const vm_size_t *regs_size, std::size_t nregs);

  /// Update many registers at once
  /// @param ids array of register ids
  /// @param in_bufs array of buffer where every new register value is read
  /// @param regs_size array of the size of each register. If all registers have
  /// the same size, you can write 0 in regs_size[1]
  /// @param nregs size of arrays
  void set_regs(const vm_reg_t *ids, const char **in_bufs,
                const vm_size_t *regs_size, std::size_t nregs);

  /// Get informations about multiple registers at once
  /// The field val is not updated
  /// @param ids array of register ids
  /// @param out_infos array of RegInfos where data will be written
  /// @param nregs size of arrays
  void get_regs_infos(const vm_reg_t *ids, RegInfos *out_infos,
                      std::size_t nregs);

  /// Find ids of many registers at once
  /// @param reg_names array of register names
  /// @param out_ids array where every register ids will be written
  /// @param nregs size of arrays
  void find_regs_ids(const char **reg_names, vm_reg_t *out_ids,
                     std::size_t nregs);

  /// Read at many memory locations at once
  /// @param src_addrs array of VM addresses where the data is read
  /// @param bufs_sizes array of the number of bytes of each read
  /// @param out_bufs array of buffers where the data will be writtem
  /// @param nbuffs size of arrays
  void read_mem(const vm_ptr_t *src_addrs, const vm_size_t *bufs_sizes,
                char **out_bufs, std::size_t nbuffs);

  /// Write at many memory locations at once
  /// @param dst_addrs array of VM addresses where the data will be written
  /// @param bufs_sizes array of of the number of bytes of each write
  /// @param in_bufs array of buffers that contains the data to be written
  /// @param nbuffs size of arrays
  void write_mem(const vm_ptr_t *dst_addrs, const vm_size_t *bufs_sizes,
                 const char **in_bufs, std::size_t nbuffs);

  /// Get infos about all symbols located at [addr, addr + size[
  /// @param out_infos vector where data will be written
  void get_symbols_by_addr(vm_ptr_t addr, vm_size_t size,
                           std::vector<SymbolInfos> &out_infos);

  /// Get infos about many symbols given their ids
  /// @param ids array of symbol ids
  /// @param out_infos array where data will be written
  /// @param nsyms size of array
  void get_symbols_by_ids(const vm_sym_t *ids, SymbolInfos *out_infos,
                          std::size_t nsyms);

  /// Get infos about many symbols given their names
  /// @param names array of symbol names
  /// @param out_infos array where data will be written
  /// @param nsyms size of array
  void get_symbols_by_names(const char **names, SymbolInfos *out_infos,
                            std::size_t nsyms);

  /// Get string representation of code
  /// @param addr VM address where to start getting codes
  /// @param nins number of instructions that need to be decoded
  /// @param out_code_size will contain the number of bytes decoded
  /// @param out_sizes will contain the size in bytes of each decoded
  /// instructions
  /// @extra vector<string> not good, can do better with cache
  void get_code_text(vm_ptr_t addr, std::size_t nins,
                     std::vector<std::string> &out_text,
                     std::vector<vm_size_t> &out_sizes);

  /// Add many breakpoint at once
  /// @param addrs array of addresses
  /// @param size array size
  void add_breakpoints(const vm_ptr_t *addrs, std::size_t size);

  /// Add many breakpoint at once
  /// @param addrs array of addresses
  /// @param size array size
  void del_breakpoints(const vm_ptr_t *addrs, std::size_t size);

  /// @EXTRA had has_breakpoint used cache

  /// Resume program execution
  void resume(ResumeType type);

  // ===== Always stored informations =====
  // These calls never block

  // === Update informations ===
  // These informations are valid until the program resume

  // @returns the address where the Debugguer is stopped
  // May differ from PC on some vms
  vm_ptr_t get_execution_point();

  /// @returns the reason why the program is stopped
  StoppedState get_stopped_state();

  /// Get the current list of calls
  CallStack get_call_stack();

  // === Static VM informations ===
  // These informations are always valid

  /// Returns a reference to the VMInfos object
  /// Valid as long as this remains valid
  const VMInfos &get_vm_infos() const { return _vm_infos; }

  /// Returns total number of registers available
  /// Some VMs may have infinite registers (usually returns huge number)
  vm_reg_t registers_count() const;

  /// List all regs of a specific kind
  /// For VM with lots of registers, usually only list the main special ones
  /// Remains valid as long as this remains valid
  const std::vector<vm_reg_t> &list_regs(RegKind kind) const;

  /// Returns the full memory size of the VM in bytes
  vm_size_t memory_size() const;

  /// Returns the total number of symbols
  vm_sym_t symbols_count() const;

  // Returns the size of a pointer type value
  vm_size_t pointer_size() const;

  // Returns the size of an int type value
  vm_size_t integer_size() const;

  // Returns true if the VM instructions have binary opcodes
  bool use_opcode() const;

private:
  std::unique_ptr<DBClientImpl> _impl;
  State _state;
  DBClientUpdate _udp;
  VMInfos _vm_infos;

  // Discard all temporary infos (eg memory / reg values)
  void _discard_tmp_cache();

  // Caching: register infos + vals
  // infos static data always valid
  // value cleared after each instruction

  // Fetch all register infos into cache
  void _fetch_reg_infos_by_id(const vm_reg_t *idxs, std::size_t nregs);

  // Fetch all register infos into cache
  void _fetch_reg_infos_by_name(const char **names, std::size_t nregs);

  // Fetch all register values into cache
  void _fetch_reg_vals_by_id(const vm_reg_t *idxs, std::size_t nregs);

  std::vector<RegInfos> _regi_arr; // all reg infos
  std::map<vm_reg_t, std::size_t>
      _regi_idx_map; // map reg index => pos in _regi_arr
  std::map<std::string, std::size_t>
      _regi_name_map; // map reg name => pos in _regi_arr
};

} // namespace odb
