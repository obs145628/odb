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
    ReqGetRegsVar req;
    req.nregs = nregs;
    req.in_ids = const_cast<vm_reg_t *>(ids);
    req.in_regs_size = const_cast<vm_size_t *>(regs_size);
    req.out_bufs = out_bufs;
    _impl->send_req(req);
  }
}

void DBClientImplData::set_regs(const vm_reg_t *ids, const char **in_bufs,
                                const vm_size_t *regs_size, std::size_t nregs) {
  if (nregs == 0)
    return;

  if (nregs < 2 || regs_size[1] == 0) {
    ReqSetRegs req;
    req.nregs = nregs;
    req.reg_size = regs_size[0];
    req.in_ids = const_cast<vm_reg_t *>(ids);
    req.in_bufs = const_cast<char **>(in_bufs);
    _impl->send_req(req);
  }

  else {
    ReqSetRegsVar req;
    req.nregs = nregs;
    req.in_ids = const_cast<vm_reg_t *>(ids);
    req.in_regs_size = const_cast<vm_size_t *>(regs_size);
    req.in_bufs = const_cast<char **>(in_bufs);
    _impl->send_req(req);
  }
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
  if (nbuffs == 0)
    return;

  ReqReadMemVar req;
  req.nbufs = nbuffs;
  req.in_addrs = const_cast<vm_ptr_t *>(src_addrs);
  req.in_bufs_size = const_cast<vm_size_t *>(bufs_sizes);
  req.out_bufs = out_bufs;
  _impl->send_req(req);
}

void DBClientImplData::write_mem(const vm_ptr_t *dst_addrs,
                                 const vm_size_t *bufs_sizes,
                                 const char **in_bufs, std::size_t nbuffs) {
  if (nbuffs == 0)
    return;

  ReqWriteMemVar req;
  req.nbufs = nbuffs;
  req.in_addrs = const_cast<vm_ptr_t *>(dst_addrs);
  req.in_bufs_size = const_cast<vm_size_t *>(bufs_sizes);
  req.in_bufs = const_cast<char **>(in_bufs);
  _impl->send_req(req);
}

void DBClientImplData::get_symbols_by_ids(const vm_sym_t *ids,
                                          SymbolInfos *out_infos,
                                          std::size_t nsyms) {
  ReqGetSymsByIds req;
  req.nsyms = nsyms;
  req.in_ids = const_cast<vm_sym_t *>(ids);
  req.out_infos = out_infos;
  _impl->send_req(req);
}

void DBClientImplData::get_symbols_by_addr(
    vm_ptr_t addr, vm_size_t size, std::vector<SymbolInfos> &out_infos) {
  ReqGetSymsByAddr req;
  req.addr = addr;
  req.size = size;
  _impl->send_req(req);
  out_infos = req.out_infos;
}

void DBClientImplData::get_symbols_by_names(const char **names,
                                            SymbolInfos *out_infos,
                                            std::size_t nsyms) {
  ReqGetSymsByNames req;
  req.nsyms = nsyms;
  req.in_names = const_cast<char **>(names);
  req.out_infos = out_infos;
  _impl->send_req(req);
}

void DBClientImplData::get_code_text(vm_ptr_t addr, std::size_t nins,
                                     std::vector<std::string> &out_text,
                                     std::vector<vm_size_t> &out_sizes) {
  ReqGetCodeText req;
  req.addr = addr;
  req.nins = nins;
  _impl->send_req(req);
  out_text = req.out_text;
  out_sizes = req.out_sizes;
}

void DBClientImplData::add_breakpoints(const vm_ptr_t *addrs,
                                       std::size_t size) {
  ReqAddBkps req;
  req.size = size;
  req.in_addrs = const_cast<vm_ptr_t *>(addrs);
  _impl->send_req(req);
}

void DBClientImplData::del_breakpoints(const vm_ptr_t *addrs,
                                       std::size_t size) {
  ReqDelBkps req;
  req.size = size;
  req.in_addrs = const_cast<vm_ptr_t *>(addrs);
  _impl->send_req(req);
}

void DBClientImplData::resume(ResumeType type) {
  ReqResume req;
  req.type = type;
  _impl->send_req(req);
}

} // namespace odb
