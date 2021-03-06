//===-- mess/fwd.hh - Forward Definitions -----------------------*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Forward definitions related to odb_core
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>

namespace odb {

class DBClient;
class DBClientImpl;
struct DBClientUpdate;
class SerialInBuff;
class SerialOutBuff;

using db_client_req_t = int;

} // namespace odb
