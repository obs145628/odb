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

  ERR = 100,
};

struct ReqConnect {
  VMInfos out_infos;
  DBClientUpdate out_udp;
};

} // namespace odb
