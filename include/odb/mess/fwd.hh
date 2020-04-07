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

using regidx_t = std::uint32_t;

enum class RegKind {
  general,
  program_counter,
  stack_pointer,
};

} // namespace odb
