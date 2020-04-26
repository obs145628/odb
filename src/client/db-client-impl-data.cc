#include "odb/client/db-client-impl-data.hh"

#include <cassert>

#include "odb/client/abstract-data-client.hh"
#include "odb/mess/request-handler.hh"
#include "odb/mess/request.hh"
#include "odb/mess/serial.hh"
#include "odb/server/debugger.hh"

namespace odb {

DBClientImplData::DBClientImplData(std::unique_ptr<AbstractDataClient> &&dc)
    : _dc(std::move(dc)) {}

void DBClientImplData::connect(VMInfos &infos, DBClientUpdate &udp) {
  if (!_dc->connect())
    throw VMApi::Error("Failed to connect to DB server");

  RequestHandler rh(false);
  ReqConnect req;

  SerialOutBuff os;
  os << ReqType::CONNECT;
  rh.client_write_request(os, req);
  if (!_dc->send_data(os))
    throw VMApi::Error("Failed to send request to DB server");

  SerialInBuff is;
  if (!_dc->recv_data(is))
    throw VMApi::Error("Failed to receive response from DB server");
  ReqType res_ty;
  is >> res_ty;
  assert(res_ty == ReqType::CONNECT);
  rh.client_read_response(is, req);

  infos = req.out_infos;
  udp = req.out_udp;
}

void DBClientImplData::stop() {}

void DBClientImplData::check_stopped(DBClientUpdate &udp) { (void)udp; }

void DBClientImplData::get_regs(const vm_reg_t *ids, char **out_bufs,
                                const vm_size_t *regs_size, std::size_t nregs) {
  (void)ids;
  (void)out_bufs;
  (void)regs_size;
  (void)nregs;
}

void DBClientImplData::set_regs(const vm_reg_t *ids, const char **in_bufs,
                                const vm_size_t *regs_size, std::size_t nregs) {
  (void)ids;
  (void)in_bufs;
  (void)regs_size;
  (void)nregs;
}

void DBClientImplData::get_regs_infos(const vm_reg_t *ids, RegInfos *out_infos,
                                      std::size_t nregs) {
  (void)ids;
  (void)out_infos;
  (void)nregs;
}

void DBClientImplData::find_regs_ids(const char **reg_names, vm_reg_t *out_ids,
                                     std::size_t nregs) {
  (void)reg_names;
  (void)out_ids;
  (void)nregs;
}

void DBClientImplData::read_mem(const vm_ptr_t *src_addrs,
                                const vm_size_t *bufs_sizes, char **out_bufs,
                                std::size_t nbuffs) {
  (void)src_addrs;
  (void)bufs_sizes;
  (void)out_bufs;
  (void)nbuffs;
}

void DBClientImplData::write_mem(const vm_ptr_t *dst_addrs,
                                 const vm_size_t *bufs_sizes,
                                 const char **in_bufs, std::size_t nbuffs) {
  (void)dst_addrs;
  (void)bufs_sizes;
  (void)in_bufs;
  (void)nbuffs;
}

void DBClientImplData::get_symbols_by_ids(const vm_sym_t *ids,
                                          SymbolInfos *out_infos,
                                          std::size_t nsyms) {
  (void)ids;
  (void)out_infos;
  (void)nsyms;
}

void DBClientImplData::get_symbols_by_addr(
    vm_ptr_t addr, vm_size_t size, std::vector<SymbolInfos> &out_infos) {
  (void)addr;
  (void)size;
  (void)out_infos;
}

void DBClientImplData::get_symbols_by_names(const char **names,
                                            SymbolInfos *out_infos,
                                            std::size_t nsyms) {
  (void)names;
  (void)out_infos;
  (void)nsyms;
}

void DBClientImplData::get_code_text(vm_ptr_t addr, std::size_t nins,
                                     std::vector<std::string> &out_text,
                                     std::vector<vm_size_t> &out_sizes) {
  (void)addr;
  (void)nins;
  (void)out_text;
  (void)out_sizes;
}

void DBClientImplData::add_breakpoints(const vm_ptr_t *addrs,
                                       std::size_t size) {
  (void)addrs;
  (void)size;
}

void DBClientImplData::del_breakpoints(const vm_ptr_t *addrs,
                                       std::size_t size) {
  (void)addrs;
  (void)size;
}

void DBClientImplData::resume(ResumeType type) { (void)type; }

} // namespace odb
