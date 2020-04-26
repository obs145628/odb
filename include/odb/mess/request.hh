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
  GET_REGS_INFOS,
  FIND_REGS_IDS,

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

struct ReqErr {
  static constexpr ReqType REQ_TYPE = ReqType::ERR;

  std::string msg;
};

} // namespace odb
