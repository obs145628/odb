#include "odb/server/db-client-impl-vmside.hh"

#include <cassert>

#include "odb/mess/db-client.hh"
#include "odb/server/debugger.hh"

namespace odb {

namespace {
bool is_running(const Debugger &db) {
  switch (db.get_state()) {
  case Debugger::State::RUNNING_TOFINISH:
  case Debugger::State::RUNNING_BKP:
  case Debugger::State::RUNNING_STEP:
  case Debugger::State::RUNNING_STEP_OVER:
  case Debugger::State::RUNNING_STEP_OUT:
    return true;
  default:
    return false;
  }
}
} // namespace

DBClientImplVMSide::DBClientImplVMSide(Debugger &db) : _db(db) {}

void DBClientImplVMSide::connect(VMInfos &infos, DBClientUpdate &udp) {
  infos = _db.get_vm_infos();
  check_stopped(udp);
}

void DBClientImplVMSide::stop() { _db.stop(); }

void DBClientImplVMSide::check_stopped(DBClientUpdate &udp) {
  if (is_running(_db)) {
    udp.stopped = false;
    return;
  }

  switch (_db.get_state()) {
  case Debugger::State::NOT_STARTED:
  case Debugger::State::STOPPED:
    udp.vm_state = StoppedState::READY;
    break;
  case Debugger::State::ERROR:
    udp.vm_state = StoppedState::ERROR;
    break;
  case Debugger::State::EXIT:
    udp.vm_state = StoppedState::EXIT;
    break;
  default:
    assert(0);
  }

  udp.stopped = true;
  udp.addr = _db.get_execution_point();
  udp.stack = _db.get_call_stack();
}

void DBClientImplVMSide::get_regs(const vm_reg_t *ids, char **out_bufs,
                                  const vm_size_t *, std::size_t nregs) {

  for (std::size_t i = 0; i < nregs; ++i) {
    auto id = ids[i];
    auto buf = out_bufs[i];
    _db.get_reg(id, reinterpret_cast<std::uint8_t *>(buf));
  }
}

void DBClientImplVMSide::set_regs(const vm_reg_t *ids, const char **in_bufs,
                                  const vm_size_t *, std::size_t nregs) {
  for (std::size_t i = 0; i < nregs; ++i) {
    auto id = ids[i];
    auto buf = in_bufs[i];
    _db.set_reg(id, reinterpret_cast<const std::uint8_t *>(buf));
  }
}

void DBClientImplVMSide::get_regs_infos(const vm_reg_t *ids,
                                        RegInfos *out_infos,
                                        std::size_t nregs) {
  for (std::size_t i = 0; i < nregs; ++i) {
    auto id = ids[i];
    out_infos[i] = _db.get_reg_infos(id);
  }
}

void DBClientImplVMSide::find_regs_ids(const char **reg_names,
                                       vm_reg_t *out_ids, std::size_t nregs) {
  for (std::size_t i = 0; i < nregs; ++i) {
    std::string name = reg_names[i];
    out_ids[i] = _db.find_reg_id(name);
  }
}

void DBClientImplVMSide::read_mem(const vm_ptr_t *src_addrs,
                                  const vm_size_t *bufs_sizes, char **out_bufs,
                                  std::size_t nbuffs) {
  for (std::size_t i = 0; i < nbuffs; ++i) {
    auto addr = src_addrs[i];
    auto len = bufs_sizes[i];
    auto buf = reinterpret_cast<std::uint8_t *>(out_bufs[i]);
    _db.read_mem(addr, len, buf);
  }
}

void DBClientImplVMSide::write_mem(const vm_ptr_t *dst_addrs,
                                   const vm_size_t *bufs_sizes,
                                   const char **in_bufs, std::size_t nbuffs) {

  for (std::size_t i = 0; i < nbuffs; ++i) {
    auto addr = dst_addrs[i];
    auto len = bufs_sizes[i];
    auto buf = reinterpret_cast<const std::uint8_t *>(in_bufs[i]);
    _db.write_mem(addr, len, buf);
  }
}

void DBClientImplVMSide::get_symbols_by_ids(const vm_sym_t *ids,
                                            SymbolInfos *out_infos,
                                            std::size_t nsyms) {
  for (std::size_t i = 0; i < nsyms; ++i) {
    out_infos[i] = _db.get_symbol_infos(ids[i]);
  }
}

void DBClientImplVMSide::get_symbols_by_addr(
    vm_ptr_t addr, vm_size_t size, std::vector<SymbolInfos> &out_infos) {
  auto ids = _db.get_symbols(addr, size);
  out_infos.resize(ids.size());
  get_symbols_by_ids(&ids[0], &out_infos[0], out_infos.size());
}

void DBClientImplVMSide::get_symbols_by_names(const char **names,
                                              SymbolInfos *out_infos,
                                              std::size_t nsyms) {
  for (std::size_t i = 0; i < nsyms; ++i) {
    std::string name = names[i];
    auto id = _db.find_sym_id(name);
    out_infos[i] = _db.get_symbol_infos(id);
  }
}

void DBClientImplVMSide::get_code_text(vm_ptr_t addr, std::size_t nins,
                                       vm_size_t &out_code_size,
                                       std::vector<std::string> &out_text) {

  vm_size_t len = 0;
  out_text.resize(nins);

  for (std::size_t i = 0; i < nins; ++i) {
    vm_size_t dist;
    out_text[i] = _db.get_code_text(addr, dist);
    addr += dist;
    len += dist;
  }

  out_code_size = len;
}

void DBClientImplVMSide::add_breakpoints(const vm_ptr_t *addrs,
                                         std::size_t size) {
  for (std::size_t i = 0; i < size; ++i)
    _db.add_breakpoint(addrs[i]);
}

void DBClientImplVMSide::del_breakpoints(const vm_ptr_t *addrs,
                                         std::size_t size) {

  for (std::size_t i = 0; i < size; ++i)
    _db.del_breakpoint(addrs[i]);
}

void DBClientImplVMSide::resume(ResumeType type) { _db.resume(type); }

} // namespace odb
