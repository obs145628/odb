//===-- server/vm-api.hh - VMApi class definition --------------*- C++//-*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Definition of the abstract class VMApi
/// Must be implemented for a VM to be usable with ODB
///
//===----------------------------------------------------------------------===//

#pragma once

#include <exception>
#include <vector>

#include "fwd.hh"

namespace odb {

/// Abstract class
/// Contain many API calls to get / set informations about the VM
/// Debuguer can talk to any VM using an instance of VMApi
///
/// All functions may throw a VMApi::Error
class VMApi {
public:
  class Error : public std::exception {

  public:
    Error(std::string msg) : _msg(msg) {}

    const char *what() const throw() { return _msg.c_str(); }

  private:
    std::string _msg;
  };

  enum class UpdateState {
    ERROR,    // The last instruction caused an error that stopped the VM
    EXIT,     // the last instruction stopped the VM
    CALL_SUB, // the last instruction  was a call to a subroutine
    RET_SUB,  // the last instruction was a return from a subroutine
    OK,       // Nothing special about last instruction
  };

  struct UpdateInfos {
    UpdateState state;
    vm_ptr_t act_addr; // address of the next instruction to be executed
  };

  VMApi() = default;
  virtual ~VMApi() = default;

  /// Called only once, before program starts, to get initial informations
  virtual VMInfos get_vm_infos() = 0;

  /// Get some information about the last executed instruction
  virtual UpdateInfos get_update_infos() = 0;

  /// Store informations about register of id `idx` in `infos`
  /// If `val_only` is true, only update the val field
  /// Otherwhise, update everything but val field
  virtual void get_reg(vm_reg_t idx, RegInfos &infos, bool val_only) = 0;

  /// Change the value of register `idx`
  virtual void set_reg(vm_reg_t idx, const std::uint8_t *new_val) = 0;

  /// Find a register id given its name
  virtual vm_reg_t find_reg_id(const std::string &name) = 0;

  /// Read `size` bytes of memory at address `addr`, and store them in `out_buf`
  virtual void read_mem(vm_ptr_t addr, vm_size_t size,
                        std::uint8_t *out_buf) = 0;

  /// Write `size` byes of data to `addr` from `buf` src
  virtual void write_mem(vm_ptr_t addr, vm_size_t size,
                         const std::uint8_t *buf) = 0;

  /// Return all symbols index defined in the range [`addr`, `addr` + `size`[
  virtual std::vector<vm_sym_t> get_symbols(vm_ptr_t addr, vm_size_t size) = 0;

  /// Returns infos about a symbol given its index `idx`
  virtual SymbolInfos get_symb_infos(vm_sym_t idx) = 0;

  /// Find a symbol id given its name
  virtual vm_sym_t find_sym_id(const std::string &name) = 0;

  /// Get the text format of the instruction or data directive at address `addr`
  /// Store in `addr_dist` the number of opcode bytes read
  /// This may be used to read instruction, but also data directives (eg .byte,
  /// .word)
  /// If the code contains a reference to a symbol, it's written '{symbol_idx}'
  virtual std::string get_code_text(vm_ptr_t addr, vm_size_t &addr_dist) = 0;
};

} // namespace odb

#pragma once
