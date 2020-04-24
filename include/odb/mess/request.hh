//===-- mess/request.hh - Request struct definition -------------*- C++ -*-===//
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

#include "../server/fwd.hh"
#include "fwd.hh"
#include "message.hh"

namespace odb {

// Get everything needed for connection init:
// vminfos, and first update
// sent by connect()
ODB_MESSAGE_STRUCT ReqInit {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 1;

  std::uint32_t tag;

  static void build(Message & req);
};

// Get update infos if VM exec stopped, or nothing if still running
// sent by check_stopped()
ODB_MESSAGE_STRUCT ReqUpdate {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 2;

  std::uint32_t tag;

  static void build(Message & req);
};

// Get reg values for all regs of same size
// sent by get_regs()
ODB_MESSAGE_STRUCT ReqGetRegs {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 3;

  std::uint16_t nregs;
  std::uint16_t reg_size;
  char data[];

  static void build(Message & req, const vm_reg_t *ids, vm_size_t reg_size,
                    std::size_t nregs);

  vm_reg_t *ids() { return reinterpret_cast<vm_reg_t *>(&data[0]); }
};

// Get reg values for regs of variable size
// sent by get_regs()
ODB_MESSAGE_STRUCT ReqGetRegsVar {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 4;

  std::uint16_t nregs;
  char data[];

  static void build(Message & req, const vm_reg_t *ids,
                    const vm_size_t *regs_size, std::size_t nregs);

  vm_reg_t *ids() { return reinterpret_cast<vm_reg_t *>(&data[0]); }

  vm_size_t *sizes() {
    return reinterpret_cast<vm_size_t *>(&data[nregs * sizeof(vm_reg_t)]);
  }
};

// Set reg values for all regs of same size
// sent by set_regs()
ODB_MESSAGE_STRUCT ReqSetRegs {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 5;

  std::uint16_t nregs;
  std::uint16_t reg_size;
  char data[];

  static void build(Message & req, const vm_reg_t *ids, const char **in_bufs,
                    vm_size_t reg_size, std::size_t nregs);

  vm_reg_t *ids() { return reinterpret_cast<vm_reg_t *>(&data[0]); }

  char *buff() {
    return reinterpret_cast<char *>(&data[nregs * sizeof(vm_reg_t)]);
  }

  char *buff(std::size_t pos) { return buff() + pos * reg_size; }
};

// Set reg values for regs of variable size
// sent by set_regs()
ODB_MESSAGE_STRUCT ReqSetRegsVar {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 6;

  std::uint16_t nregs;
  char data[];

  static void build(Message & req, const vm_reg_t *ids, const char **in_bufs,
                    const vm_size_t *reg_size, std::size_t nregs);

  vm_reg_t *ids() { return reinterpret_cast<vm_reg_t *>(&data[0]); }

  char *buff() {
    return reinterpret_cast<char *>(&data[nregs * sizeof(vm_reg_t)]);
  }
};

// Get reg infos for many regs
// sent by get_regs_infos()
ODB_MESSAGE_STRUCT ReqGetRegsInfos {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 7;

  std::uint16_t nregs;
  char data[];

  static void build(Message & req, const vm_reg_t *ids, std::size_t nregs);

  vm_reg_t *ids() { return reinterpret_cast<vm_reg_t *>(&data[0]); }
};

// Find reg ids given multiple reg names
// sent by find_regs_ids()
ODB_MESSAGE_STRUCT ReqGetRegsIds {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 8;

  std::uint16_t nregs;
  char data[];

  static void build(Message & req, const char **regs_names, std::size_t nregs);

  char *names() { return reinterpret_cast<char *>(&data[0]); }
};

// Read memory at multiple locations at the same time
// Sent by read_mem
ODB_MESSAGE_STRUCT ReqReadMem {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 9;

  std::uint16_t nreads;
  char data[];

  static void build(Message & req, const vm_ptr_t *src_addrs,
                    const vm_size_t *bufs_sizes, std::size_t nread);

  vm_ptr_t *src_addrs() { return reinterpret_cast<vm_ptr_t *>(&data[0]); }

  vm_size_t *bufs_sizes() {
    return reinterpret_cast<vm_size_t *>(&data[nreads * sizeof(vm_ptr_t)]);
  }
};

// Write memory at multiple locations at the same time
// Sent by write_mem
ODB_MESSAGE_STRUCT ReqWriteMem {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 10;

  std::uint16_t nwrites;
  char data[];

  static void build(Message & req, const vm_ptr_t *dst_addrs,
                    const vm_size_t *bufs_sizes, const char **in_bufs,
                    std::size_t nwrites);

  vm_ptr_t *dst_addrs() { return reinterpret_cast<vm_ptr_t *>(&data[0]); }

  vm_size_t *bufs_sizes() {
    return reinterpret_cast<vm_size_t *>(&data[nwrites * sizeof(vm_ptr_t)]);
  }

  char *buff() {
    return reinterpret_cast<char *>(
        &data[nwrites * (sizeof(vm_ptr_t) + sizeof(vm_size_t))]);
  }
};

// Find symbol infos given their ids
// Sent by get_symbols_by_ids
ODB_MESSAGE_STRUCT ReqGetSymbsById {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 11;

  std::uint16_t nsyms;
  char data[];

  static void build(Message & req, const vm_sym_t *ids, std::size_t nsyms);

  vm_sym_t *ids() { return reinterpret_cast<vm_sym_t *>(&data[0]); }
};

// Find symbol infos in a given addr range
// Sent by get_symbols_by_addr
ODB_MESSAGE_STRUCT ReqGetSymbsByAddr {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 12;

  vm_ptr_t addr;
  vm_size_t size;

  static void build(Message & req, vm_ptr_t addr, vm_size_t size);
};

// Find symbol infos given their names
// Sent by get_symbols_by_names
ODB_MESSAGE_STRUCT ReqGetSymbsByNames {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 13;

  std::uint16_t nsyms;
  char data[];

  static void build(Message & req, const char **names, std::size_t nsyms);

  char *names() { return reinterpret_cast<char *>(&data[0]); }
};

// Add / remove breakpoints
// Sent by add_breakpoints / del_breakpoints
ODB_MESSAGE_STRUCT ReqEditBkps {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 14;

  std::uint16_t add_count;
  std::uint16_t del_count;
  char data[];

  static void build(Message & req, const vm_ptr_t *add_addrs,
                    const vm_ptr_t *del_addrs, std::size_t add_count,
                    std::size_t del_count);

  vm_ptr_t *add_addrs() { return reinterpret_cast<vm_ptr_t *>(&data[0]); }

  vm_ptr_t *del_addrs() {
    return reinterpret_cast<vm_ptr_t *>(&data[add_count * sizeof(vm_ptr_t)]);
  }
};

// Resume program execution
// Sent by resume
ODB_MESSAGE_STRUCT ReqEditResume {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 15;

  std::uint8_t type;

  static void build(Message & req, ResumeType type);

  ResumeType get_type();
};

} // namespace odb
