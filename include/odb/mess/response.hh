//===-- mess/response.hh - Response struct definition -----------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Response struct represent data sent by the server to a client, to respond to
/// the request received from the client
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../server/fwd.hh"
#include "fwd.hh"
#include "message.hh"

namespace odb {

// An error happened
// can be sent to any
ODB_MESSAGE_STRUCT ResErr {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 1;

  std::int16_t code;
  char data[];

  static void build(Message & res, int code, const char *mess);
};

// Only a simple status value for short answer
// Sent to check_stopped, ???
ODB_MESSAGE_STRUCT ResStatus {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 2;

  std::uint32_t status;

  static void build(Message & res, unsigned status);
};

// ALL VM Infos
// Never sent alone
ODB_MESSAGE_STRUCT ResVMInfos {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 3;

  vm_reg_t regs_count;
  vm_reg_t regs_general_count;
  vm_reg_t regs_program_counter_count;
  vm_reg_t regs_stack_pointer_count;
  vm_reg_t regs_base_pointer_count;
  vm_reg_t regs_flags_count;
  vm_size_t memory_size;
  vm_sym_t symbols_count;
  vm_size_t pointer_size;
  vm_size_t integer_size;
  std::uint8_t use_opcode;

  char data[];

  void get_infos(VMInfos & out_infos);

  static void build(Message & res, const VMInfos &infos);
};

// VM Update
// Sento to check_stopped()
ODB_MESSAGE_STRUCT ResUpdate {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 4;

  std::uint8_t stopped_state;
  vm_ptr_t addr; // execution point
  char data[];

  void get_udp(DBClientUpdate & out_udp);

  static void build(Message & res, const DBClientUpdate &udp);
};

// VM Infos + VM Update
// Sent to connect()
ODB_MESSAGE_STRUCT ResVMInfosUpdate {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 5;

  std::uint16_t tag;
  char data[];

  void get_infos(VMInfos & out_infos);
  void get_udp(DBClientUpdate & out_udp);

  static void build(Message & res, const VMInfos &infos,
                    const DBClientUpdate &udp);
};

// A group of multiple buffers of same size
// Sent to ???
ODB_MESSAGE_STRUCT ResBuffs {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 6;

  std::uint16_t nb_buffs;
  std::uint32_t buffs_size;
  char data[];

  char *buff() { return reinterpret_cast<char *>(&data[0]); }

  static void build(Message & res, std::size_t nb_buffs, std::size_t buffs_size,
                    const char **buffs);
};

// A group of multiple buffers of variable size
// Sent to ???
ODB_MESSAGE_STRUCT ResBuffsVar {
  static constexpr Message::typeid_t MESSAGE_TYPEID = 7;

  std::uint16_t nb_buffs;
  char data[];

  std::uint32_t *buffs_sizes() {
    return reinterpret_cast<std::uint32_t *>(&data[0]);
  }
  char *buff() {
    return reinterpret_cast<char *>(&data[nb_buffs * sizeof(std::uint32_t)]);
  }

  static void build(Message & res, std::size_t nb_buffs,
                    std::uint32_t * buffs_size, const char **buffs);
};

} // namespace odb
