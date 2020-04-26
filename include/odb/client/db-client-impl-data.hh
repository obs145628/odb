//===-- client/db-client-impl-data.hh - DBClientImplData class-*- C++ ---*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Implementation of DBClient that communicate with serialbuffs
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../mess/db-client-impl.hh"
#include "fwd.hh"

#include <memory>

namespace odb {

/// Implementation for the DBClient
/// Send/recv serialbuff objects
/// Serialize commands
/// Use an abstract dataclient responsible for recv/send serialbuffs using any
/// protocol
class DBClientImplData : public DBClientImpl {
public:
  DBClientImplData(std::unique_ptr<AbstractDataClient> &&dc);

  void connect(VMInfos &infos, DBClientUpdate &udp) override;

  void stop() override;

  void check_stopped(DBClientUpdate &udp) override;

  void get_regs(const vm_reg_t *ids, char **out_bufs,
                const vm_size_t *regs_size, std::size_t nregs) override;

  void set_regs(const vm_reg_t *ids, const char **in_bufs,
                const vm_size_t *regs_size, std::size_t nregs) override;

  void get_regs_infos(const vm_reg_t *ids, RegInfos *out_infos,
                      std::size_t nregs) override;

  void find_regs_ids(const char **reg_names, vm_reg_t *out_ids,
                     std::size_t nregs) override;

  void read_mem(const vm_ptr_t *src_addrs, const vm_size_t *bufs_sizes,
                char **out_bufs, std::size_t nbuffs) override;

  void write_mem(const vm_ptr_t *dst_addrs, const vm_size_t *bufs_sizes,
                 const char **in_bufs, std::size_t nbuffs) override;

  void get_symbols_by_ids(const vm_sym_t *ids, SymbolInfos *out_infos,
                          std::size_t nsyms) override;

  void get_symbols_by_addr(vm_ptr_t addr, vm_size_t size,
                           std::vector<SymbolInfos> &out_infos) override;

  void get_symbols_by_names(const char **names, SymbolInfos *out_infos,
                            std::size_t nsyms) override;

  void get_code_text(vm_ptr_t addr, std::size_t nins,
                     std::vector<std::string> &out_text,
                     std::vector<vm_size_t> &out_sizes) override;

  void add_breakpoints(const vm_ptr_t *addrs, std::size_t size) override;

  void del_breakpoints(const vm_ptr_t *addrs, std::size_t size) override;

  void resume(ResumeType type) override;

private:
  std::unique_ptr<AbstractDataClient> _dc;
};

} // namespace odb
