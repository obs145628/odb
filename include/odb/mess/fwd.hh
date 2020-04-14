//===-- mess/fwd.hh - Forward Definitions ----------------------*- C++//-*-===//
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

struct Request;
struct Response;

class DBClient;
class DBClientImpl;
struct DBClientUpdate;

using db_client_req_t = int;

} // namespace odb
