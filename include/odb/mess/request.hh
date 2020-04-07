//===-- mess/request.hh - Request struct definition ------------*- C++//-*-===//
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
#include <memory>
#include <vector>

namespace odb {

class Request {

public:
  enum class Type {
    get_reg,
    get_reg_infos,
    set_reg,
    get_special_reg,
  };

  struct ReqGetReg;
  struct ReqGetRegInfos;
  struct ReqSetReg;
  struct ReqGetSpecialReg;

  class Visitor {
  public:
    virtual void visit(const ReqGetReg &req) = 0;
    virtual void visit(const ReqGetRegInfos &req) = 0;
    virtual void visit(const ReqSetReg &req) = 0;
    virtual void visit(const ReqGetSpecialReg &req) = 0;
  };

  struct ReqBase {
    ReqBase() = default;
    ReqBase(const ReqBase &) = delete;
    ReqBase &operator=(const ReqBase &) = delete;
    virtual ~ReqBase() = default;
    virtual void accept(Visitor &v) const = 0;
  };

  // Read the value of register `idx`
  // Registers can have any size
  // Response is bytes_array
  struct ReqGetReg : public ReqBase {
    regidx_t idx;
    ReqGetReg(regidx_t idx) : idx(idx) {}
    void accept(Visitor &v) const override { v.visit(*this); }
  };

  // Returns static infos about a register (name, size, datatype, is special ?)
  // Response is reg_infos
  struct ReqGetRegInfos : public ReqBase {
    regidx_t idx;
    ReqGetRegInfos(regidx_t idx) : idx(idx) {}
    void accept(Visitor &v) const override { v.visit(*this); }
  };

  // Change the value of register `idx`
  // Reponse is success
  struct ReqSetReg : public ReqBase {
    regidx_t idx;
    std::vector<std::uint8_t> bytes;
    ReqSetReg(regidx_t idx, const std::vector<std::uint8_t> &bytes)
        : idx(idx), bytes(bytes) {}
    void accept(Visitor &v) const override { v.visit(*this); }
  };

  // Returns id of a special kind of register, or -1 if the VM doesn't have it
  // Response is ids_array
  struct ReqGetSpecialReg : public ReqBase {
    RegKind kind;
    ReqGetSpecialReg(RegKind kind) : kind(kind) {}
    void accept(Visitor &v) const override { v.visit(*this); }
  };

  Request(const Request &) = delete;
  Request(Request &&) = default;

  template <class T> const T &get() {
    return dynamic_cast<const T &>(_data.get());
  }

  static Request make_get_reg(regidx_t idx) {
    return Request(Type::get_reg, std::make_unique<ReqGetReg>(idx));
  }

  static Request make_get_reg_infos(regidx_t idx) {
    return Request(Type::get_reg_infos, std::make_unique<ReqGetRegInfos>(idx));
  }

  static Request make_set_reg(regidx_t idx,
                              const std::vector<std::uint8_t> &bytes) {
    return Request(Type::set_reg, std::make_unique<ReqSetReg>(idx, bytes));
  }

  static Request make_get_special_reg(RegKind kind) {
    return Request(Type::get_special_reg,
                   std::make_unique<ReqGetSpecialReg>(kind));
  }

private:
  Request(Type type, std::unique_ptr<ReqBase> &&data)
      : _type(type), _data(std::move(data)) {}

  Type _type;
  std::unique_ptr<ReqBase> _data;
};

} // namespace odb
