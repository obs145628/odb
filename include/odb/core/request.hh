//===-- core/request.hh - Request struct definition ------------*- C++//-*-===//
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

#include "fwd.hh"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace odb {

struct Request {

  // This array alway owns the data
private:
  template <class T> struct Array {
    T *ptr;
    std::size_t len;
  };

  template <class T> Array<T> make_arr(const T *begin, const T *end) {
    Array<T> res;
    res.len = end - begin;
    res.ptr = new T[res.len];
    std::copy(begin, end, res.ptr);
    return res;
  }

  template <class T> void free_arr(const Array<T> &arr) { delete[] arr.ptr; }

public:
  enum class Type {
    get_reg,
    get_reg_infos,
    set_reg,
    get_special_reg,
  };

  enum class RegKind {
    general,
    program_counter,
    stack_pointer,
  };

  // Read the value of register `idx`
  // Registers can have any size
  // Response is bytes_array
  struct ReqGetReg {
    std::uint32_t idx;
  };

  // Returns static infos about a register (name, size, datatype, is special ?)
  // Response is reg_infos
  struct ReqGetRegInfos {
    std::uint32_t idx;
  };

  // Change the value of register `idx`
  // Reponse is success
  struct ReqSetReg {
    std::uint32_t idx;
    Array<std::uint8_t> bytes;
  };

  // Returns id of a special kind of register, or -1 if the VM doesn't have it
  // Response is ids_array
  struct ReqGetSpecialReg {
    RegKind kind;
  };

  Type type;
  union {
    ReqGetReg r_get_reg;
    ReqGetRegInfos r_get_reg_infos;
    ReqSetReg r_set_reg;
    ReqGetSpecialReg r_special_reg;
  };
};

} // namespace odb
