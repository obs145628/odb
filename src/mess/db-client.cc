#include "odb/mess/db-client.hh"

#include <cassert>

#include "odb/mess/db-client-impl.hh"
#include "odb/server/vm-api.hh"

namespace odb {

DBClient::DBClient(std::unique_ptr<DBClientImpl> &&impl)
    : _impl(std::move(impl)), _state(State::NOT_CONNECTED) {}

void DBClient::connect() {
  assert(_state == State::NOT_CONNECTED);
  _impl->connect(_vm_infos, _udp);

  // no throws means successfull connection
  if (_udp.stopped)
    _state = State::VM_STOPPED;
  else
    _state = State::VM_RUNNING;
}

void DBClient::stop() {
  assert(_state == State::VM_RUNNING);
  _impl->stop();
  _state = State::VM_STOPPED;
}

void DBClient::check_stopped() {
  assert(_state == State::VM_RUNNING);
  _impl->check_stopped(_udp);

  if (_udp.stopped)
    _state = State::VM_STOPPED;
  else
    _state = State::VM_RUNNING;
}

void DBClient::get_regs(const vm_reg_t *ids, char **out_bufs,
                        const vm_size_t *regs_size, std::size_t nregs) {
  assert(_state == State::VM_STOPPED);
  _impl->get_regs(ids, out_bufs, regs_size, nregs);
}

void DBClient::set_regs(const vm_reg_t *ids, const char **in_bufs,
                        const vm_size_t *regs_size, std::size_t nregs) {
  assert(_state == State::VM_STOPPED);
  _impl->set_regs(ids, in_bufs, regs_size, nregs);
}

void DBClient::get_regs_infos(const vm_reg_t *ids, RegInfos *out_infos,
                              std::size_t nregs) {
  assert(_state == State::VM_STOPPED);
  _impl->get_regs_infos(ids, out_infos, nregs);
}

void DBClient::find_regs_ids(const char **reg_names, vm_reg_t *out_ids,
                             std::size_t nregs) {
  assert(_state == State::VM_STOPPED);
  _impl->find_regs_ids(reg_names, out_ids, nregs);
}

void DBClient::read_mem(const vm_ptr_t *src_addrs, const vm_size_t *bufs_sizes,
                        char **out_bufs, std::size_t nbuffs) {
  assert(_state == State::VM_STOPPED);
  _impl->read_mem(src_addrs, bufs_sizes, out_bufs, nbuffs);
}

void DBClient::write_mem(const vm_ptr_t *dst_addrs, const vm_size_t *bufs_sizes,
                         const char **in_bufs, std::size_t nbuffs) {
  assert(_state == State::VM_STOPPED);
  _impl->write_mem(dst_addrs, bufs_sizes, in_bufs, nbuffs);
}

void DBClient::get_symbols_by_addr(vm_ptr_t addr, vm_size_t size,
                                   std::vector<SymbolInfos> &out_infos) {
  assert(_state == State::VM_STOPPED);
  _impl->get_symbols_by_addr(addr, size, out_infos);
}

void DBClient::get_symbols_by_ids(const vm_sym_t *ids, SymbolInfos *out_infos,
                                  std::size_t nsyms) {
  assert(_state == State::VM_STOPPED);
  _impl->get_symbols_by_ids(ids, out_infos, nsyms);
}

void DBClient::get_symbols_by_names(const char **names, SymbolInfos *out_infos,
                                    std::size_t nsyms) {
  assert(_state == State::VM_STOPPED);
  _impl->get_symbols_by_names(names, out_infos, nsyms);
}

void DBClient::get_code_text(vm_ptr_t addr, std::size_t nins,
                             vm_size_t &out_code_size,
                             std::vector<std::string> &out_text) {
  assert(_state == State::VM_STOPPED);
  _impl->get_code_text(addr, nins, out_code_size, out_text);
}

void DBClient::add_breakpoints(const vm_ptr_t *addrs, std::size_t size) {
  assert(_state == State::VM_STOPPED);
  _impl->add_breakpoints(addrs, size);
}

void DBClient::del_breakpoints(const vm_ptr_t *addrs, std::size_t size) {
  assert(_state == State::VM_STOPPED);
  _impl->del_breakpoints(addrs, size);
}

void DBClient::resume(ResumeType type) {
  assert(_state == State::VM_STOPPED);
  _impl->resume(type);
  _state = State::VM_RUNNING;
}

vm_ptr_t DBClient::get_execution_point() {
  assert(_state == State::VM_STOPPED);
  return _udp.addr;
}

StoppedState DBClient::get_stopped_state() {
  assert(_state == State::VM_STOPPED);
  return _udp.vm_state;
}

CallStack DBClient::get_call_stack() {
  assert(_state == State::VM_STOPPED);
  return _udp.stack;
}

vm_reg_t DBClient::registers_count() {
  assert(_state == State::VM_STOPPED);
  return _vm_infos.regs_count;
}

const std::vector<vm_reg_t> &DBClient::list_regs(RegKind kind) {
  assert(_state == State::VM_STOPPED);
  if (kind == RegKind::general)
    return _vm_infos.regs_general;
  else if (kind == RegKind::program_counter)
    return _vm_infos.regs_program_counter;
  else if (kind == RegKind::stack_pointer)
    return _vm_infos.regs_stack_pointer;
  else if (kind == RegKind::base_pointer)
    return _vm_infos.regs_base_pointer;
  else if (kind == RegKind::flags)
    return _vm_infos.regs_flags;
  else
    throw VMApi::Error("list_regs: invalid reg kind");
}

vm_size_t DBClient::memory_size() {
  assert(_state == State::VM_STOPPED);
  return _vm_infos.memory_size;
}

vm_sym_t DBClient::symbols_count() {
  assert(_state == State::VM_STOPPED);
  return _vm_infos.symbols_count;
}

} // namespace odb
