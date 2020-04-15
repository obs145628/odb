//===-- mess/db-client-impl.hh - DBClientImpl class definition --*- C++ -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Dynamic part of DBClient, Abstract class that need to be implemented for any
/// kind of clients
///
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>

#include "../server/fwd.hh"
#include "fwd.hh"

namespace odb {

class DBClientImpl {
public:
  DBClientImpl() = default;
  DBClientImpl(const DBClientImpl &) = delete;
  virtual ~DBClientImpl() = default;

  /// Fill `infos` with static VM infos
  /// Fill udp with first update, or set stopped = false if not stopped
  virtual void connect(VMInfos &infos, DBClientUpdate &udp) = 0;

  virtual void stop() = 0;

  /// Fill udp with update, or set stopped = false if not stopped
  virtual void check_stopped(DBClientUpdate &udp) = 0;

  virtual void get_regs(const vm_reg_t *ids, char **out_bufs,
                        const vm_size_t *regs_size, std::size_t nregs) = 0;

  virtual void set_regs(const vm_reg_t *ids, const char **in_bufs,
                        const vm_size_t *regs_size, std::size_t nregs) = 0;

  virtual void get_regs_infos(const vm_reg_t *ids, RegInfos *out_infos,
                              std::size_t nregs) = 0;

  virtual void find_regs_ids(const char **reg_names, vm_reg_t *out_ids,
                             std::size_t nregs) = 0;

  virtual void read_mem(const vm_ptr_t *src_addrs, const vm_size_t *bufs_sizes,
                        char **out_bufs, std::size_t nbuffs) = 0;

  virtual void write_mem(const vm_ptr_t *dst_addrs, const vm_size_t *bufs_sizes,
                         const char **in_bufs, std::size_t nbuffs) = 0;

  virtual void get_symbols_by_ids(const vm_sym_t *ids, SymbolInfos *out_infos,
                                  std::size_t nsyms) = 0;

  virtual void get_symbols_by_addr(vm_ptr_t addr, vm_size_t size,
                                   std::vector<SymbolInfos> &out_infos) = 0;

  virtual void get_symbols_by_names(const char **names, SymbolInfos *out_infos,
                                    std::size_t nsyms) = 0;

  virtual void get_code_text(vm_ptr_t addr, std::size_t nins,
                             vm_size_t &out_code_size,
                             std::vector<std::string> &out_text) = 0;

  virtual void add_breakpoints(const vm_ptr_t *addrs, std::size_t size) = 0;

  virtual void del_breakpoints(const vm_ptr_t *addrs, std::size_t size) = 0;

  virtual void resume(ResumeType type) = 0;
};

} // namespace odb
