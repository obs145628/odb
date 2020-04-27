//===-- mess/request.hh - Request struct definition -------------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Request struct represent request needed for DB clients
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../server/fwd.hh"
#include "db-client.hh"
#include "fwd.hh"

namespace odb {

enum class ReqType {
  CONNECT = 0,
  STOP,
  CHECK_STOPPED,
  GET_REGS,
  GET_REGS_VAR,
  SET_REGS,
  SET_REGS_VAR,
  GET_REGS_INFOS,
  FIND_REGS_IDS,
  READ_MEM,
  READ_MEM_VAR,
  WRITE_MEM,
  WRITE_MEM_VAR,
  GET_SYMS_BY_IDS,
  GET_SYMS_BY_ADDR,
  GET_SYMS_BY_NAMES,
  GET_CODE_TEXT,
  ADD_BKPS,
  DEL_BKPS,
  RESUME,

  ERR = 100,
};

struct ReqConnect {
  static constexpr ReqType REQ_TYPE = ReqType::CONNECT;

  VMInfos out_infos;
  DBClientUpdate out_udp;
};

struct ReqStop {
  static constexpr ReqType REQ_TYPE = ReqType::STOP;

  int tag;
};

struct ReqCheckStopped {
  static constexpr ReqType REQ_TYPE = ReqType::CHECK_STOPPED;

  DBClientUpdate out_udp;
};

// Get registers, all with the same size
struct ReqGetRegs {
  static constexpr ReqType REQ_TYPE = ReqType::GET_REGS;

  std::uint16_t nregs;
  vm_size_t reg_size;
  vm_reg_t *ids;
  char **out_bufs;
};

// Get registers when they have different sizes
struct ReqGetRegsVar {
  static constexpr ReqType REQ_TYPE = ReqType::GET_REGS_VAR;

  std::uint16_t nregs;
  vm_reg_t *in_ids;
  vm_size_t *in_regs_size;
  char **out_bufs;
};

// Set registers, all with the same size
struct ReqSetRegs {
  static constexpr ReqType REQ_TYPE = ReqType::SET_REGS;

  std::uint16_t nregs;
  vm_size_t reg_size;
  vm_reg_t *in_ids;
  char **in_bufs;
};

// Set registers when they have different sizes
struct ReqSetRegsVar {
  static constexpr ReqType REQ_TYPE = ReqType::SET_REGS_VAR;

  std::uint16_t nregs;
  vm_reg_t *in_ids;
  vm_size_t *in_regs_size;
  char **in_bufs;
};

struct ReqGetRegsInfos {
  static constexpr ReqType REQ_TYPE = ReqType::GET_REGS_INFOS;

  std::uint16_t nregs;
  vm_reg_t *ids;
  RegInfos *out_infos;
};

struct ReqFindRegsIds {
  static constexpr ReqType REQ_TYPE = ReqType::FIND_REGS_IDS;

  std::uint16_t nregs;
  char **in_bufs;
  vm_reg_t *out_ids;
};

// Read memory chunks, all with the same size
struct ReqReadMem {
  static constexpr ReqType REQ_TYPE = ReqType::READ_MEM;

  std::uint16_t nbufs;
  vm_size_t buf_size;
  vm_ptr_t *in_addrs;
  char **out_bufs;
};

// Read memory chunks, all with different sizes
struct ReqReadMemVar {
  static constexpr ReqType REQ_TYPE = ReqType::READ_MEM_VAR;

  std::uint16_t nbufs;
  vm_ptr_t *in_addrs;
  vm_size_t *in_bufs_size;
  char **out_bufs;
};

// Write memory chunks, all with the same size
struct ReqWriteMem {
  static constexpr ReqType REQ_TYPE = ReqType::WRITE_MEM;

  std::uint16_t nbufs;
  vm_size_t buf_size;
  vm_ptr_t *in_addrs;
  char **in_bufs;
};

// Write memory chunks, all with different sizes
struct ReqWriteMemVar {
  static constexpr ReqType REQ_TYPE = ReqType::WRITE_MEM_VAR;

  std::uint16_t nbufs;
  vm_ptr_t *in_addrs;
  vm_size_t *in_bufs_size;
  char **in_bufs;
};

struct ReqGetSymsByIds {
  static constexpr ReqType REQ_TYPE = ReqType::GET_SYMS_BY_IDS;

  std::uint16_t nsyms;
  vm_sym_t *in_ids;
  SymbolInfos *out_infos;
};

struct ReqGetSymsByAddr {
  static constexpr ReqType REQ_TYPE = ReqType::GET_SYMS_BY_ADDR;

  vm_ptr_t addr;
  vm_size_t size;
  std::vector<SymbolInfos> out_infos;
};

struct ReqGetSymsByNames {
  static constexpr ReqType REQ_TYPE = ReqType::GET_SYMS_BY_NAMES;

  std::uint16_t nsyms;
  char **in_names;
  SymbolInfos *out_infos;
};

struct ReqGetCodeText {
  static constexpr ReqType REQ_TYPE = ReqType::GET_CODE_TEXT;

  vm_ptr_t addr;
  vm_size_t nins;
  std::vector<std::string> out_text;
  std::vector<vm_size_t> out_sizes;
};

struct ReqAddBkps {
  static constexpr ReqType REQ_TYPE = ReqType::ADD_BKPS;

  std::uint16_t size;
  vm_ptr_t *in_addrs;
};

struct ReqDelBkps {
  static constexpr ReqType REQ_TYPE = ReqType::DEL_BKPS;

  std::uint16_t size;
  vm_ptr_t *in_addrs;
};

struct ReqResume {
  static constexpr ReqType REQ_TYPE = ReqType::RESUME;

  ResumeType type;
};

struct ReqErr {
  static constexpr ReqType REQ_TYPE = ReqType::ERR;

  std::string msg;
};

} // namespace odb
