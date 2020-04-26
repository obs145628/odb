#include "odb/client/db-client-impl-data.hh"

#include <cassert>

#include "odb/client/abstract-data-client.hh"
#include "odb/mess/request-handler.hh"
#include "odb/mess/request.hh"
#include "odb/mess/serial.hh"
#include "odb/server/debugger.hh"

namespace odb {

struct DBClientImplData_Internal {
public:
  DBClientImplData_Internal(std::unique_ptr<AbstractDataClient> &&dc)
      : _dc(std::move(dc)), _rh(false) {}

  void connect() {
    if (!_dc->connect())
      throw VMApi::Error("Failed to connect to DB server");
  }

  void handle_err() {
    ReqErr err;
    _rh.client_read_response(_is, err);
    throw VMApi::Error(err.msg);
  }

  template <class T> void send_req(T &req) {
    _os.reset();
    _os << T::REQ_TYPE;
    _rh.client_write_request(_os, req);
    if (!_dc->send_data(_os))
      throw VMApi::Error("Failed to send request to DB server");

    if (!_dc->recv_data(_is))
      throw VMApi::Error("Failed to receive response from DB server");
    ReqType res_ty;
    _is >> res_ty;
    if (res_ty == ReqType::ERR)
      handle_err();
    assert(res_ty == T::REQ_TYPE);
    _rh.client_read_response(_is, req);
  }

private:
  std::unique_ptr<AbstractDataClient> _dc;
  RequestHandler _rh;
  SerialInBuff _is;
  SerialOutBuff _os;
};

DBClientImplData::DBClientImplData(std::unique_ptr<AbstractDataClient> &&dc)
    : _impl(std::make_unique<DBClientImplData_Internal>(std::move(dc))) {}

DBClientImplData::~DBClientImplData() {}

void DBClientImplData::connect(VMInfos &infos, DBClientUpdate &udp) {
  _impl->connect();

  ReqConnect req;
  _impl->send_req(req);
  infos = req.out_infos;
  udp = req.out_udp;
}

void DBClientImplData::stop() {
  ReqStop req;
  _impl->send_req(req);
}

void DBClientImplData::check_stopped(DBClientUpdate &udp) {
  ReqCheckStopped req;
  _impl->send_req(req);
  udp = req.out_udp;
}

void DBClientImplData::get_regs(const vm_reg_t *ids, char **out_bufs,
                                const vm_size_t *regs_size, std::size_t nregs) {
  if (nregs == 0)
    return;

  if (nregs < 2 || regs_size[1] == 0) {
    ReqGetRegs req;
    req.nregs = nregs;
    req.reg_size = regs_size[0];
    req.ids = const_cast<vm_reg_t *>(ids);
    req.out_bufs = out_bufs;
    _impl->send_req(req);
  }

  else {
    assert(0);
  }

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
  assert(0);
}

void DBClientImplData::get_regs_infos(const vm_reg_t *ids, RegInfos *out_infos,
                                      std::size_t nregs) {
  ReqGetRegsInfos req;
  req.nregs = nregs;
  req.ids = const_cast<vm_reg_t *>(ids);
  req.out_infos = out_infos;
  _impl->send_req(req);
}

void DBClientImplData::find_regs_ids(const char **reg_names, vm_reg_t *out_ids,
                                     std::size_t nregs) {
  ReqFindRegsIds req;
  req.nregs = nregs;
  req.in_bufs = const_cast<char **>(reg_names);
  req.out_ids = out_ids;
  _impl->send_req(req);
}

void DBClientImplData::read_mem(const vm_ptr_t *src_addrs,
                                const vm_size_t *bufs_sizes, char **out_bufs,
                                std::size_t nbuffs) {
  (void)src_addrs;
  (void)bufs_sizes;
  (void)out_bufs;
  (void)nbuffs;
  assert(0);
}

void DBClientImplData::write_mem(const vm_ptr_t *dst_addrs,
                                 const vm_size_t *bufs_sizes,
                                 const char **in_bufs, std::size_t nbuffs) {
  (void)dst_addrs;
  (void)bufs_sizes;
  (void)in_bufs;
  (void)nbuffs;
  assert(0);
}

void DBClientImplData::get_symbols_by_ids(const vm_sym_t *ids,
                                          SymbolInfos *out_infos,
                                          std::size_t nsyms) {
  (void)ids;
  (void)out_infos;
  (void)nsyms;
  assert(0);
}

void DBClientImplData::get_symbols_by_addr(
    vm_ptr_t addr, vm_size_t size, std::vector<SymbolInfos> &out_infos) {
  (void)addr;
  (void)size;
  (void)out_infos;
  assert(0);
}

void DBClientImplData::get_symbols_by_names(const char **names,
                                            SymbolInfos *out_infos,
                                            std::size_t nsyms) {
  (void)names;
  (void)out_infos;
  (void)nsyms;
  assert(0);
}

void DBClientImplData::get_code_text(vm_ptr_t addr, std::size_t nins,
                                     std::vector<std::string> &out_text,
                                     std::vector<vm_size_t> &out_sizes) {
  (void)addr;
  (void)nins;
  (void)out_text;
  (void)out_sizes;
  assert(0);
}

void DBClientImplData::add_breakpoints(const vm_ptr_t *addrs,
                                       std::size_t size) {
  (void)addrs;
  (void)size;
  assert(0);
}

void DBClientImplData::del_breakpoints(const vm_ptr_t *addrs,
                                       std::size_t size) {
  (void)addrs;
  (void)size;
  assert(0);
}

void DBClientImplData::resume(ResumeType type) {
  (void)type;
  assert(0);
}

} // namespace odb
