//===-- mess/response.hh - Response struct definition ----------*- C++//-*-===//
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

#include "fwd.hh"
#include <string>

namespace odb {

struct Response {
  enum class Type { reg_infos };

  struct ResRegInfos {
    uint32_t idx;
    uint32_t size;    // number of bytes
    std::string name; // unique identifier
    RegKind kind;
  };
};

} // namespace odb
