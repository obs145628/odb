#include "odb/mess/db-client.hh"

#include <cassert>
#include <cstring>

#include <iostream>

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

  if (_udp.stopped) {
    _state = State::VM_STOPPED;
    _discard_tmp_cache();
  } else
    _state = State::VM_RUNNING;
}

void DBClient::get_regs(const vm_reg_t *ids, char **out_bufs,
                        const vm_size_t *regs_size, std::size_t nregs) {
  assert(_state == State::VM_STOPPED);
  _fetch_reg_vals_by_id(ids, nregs);

  for (std::size_t i = 0; i < nregs; ++i) {
    std::size_t reg_size =
        nregs > 1 && regs_size[1] == 0 ? regs_size[0] : regs_size[i];
    auto it = _regi_idx_map.find(ids[i]);
    assert(it != _regi_idx_map.end());
    const auto &reg = _regi_arr[it->second];
    assert(!reg.val.empty());
    std::memcpy(out_bufs[i], &reg.val[0], reg_size);
    // @EXTRA: reg_size param could be useless ? or could be asserted to check
    // size ?
  }
}

void DBClient::set_regs(const vm_reg_t *ids, const char **in_bufs,
                        const vm_size_t *regs_size, std::size_t nregs) {
  assert(_state == State::VM_STOPPED);
  _fetch_reg_infos_by_id(ids, nregs);

  _impl->set_regs(ids, in_bufs, regs_size, nregs);

  // Also update cache value
  for (std::size_t i = 0; i < nregs; ++i) {
    std::size_t reg_size =
        nregs > 1 && regs_size[1] == 0 ? regs_size[0] : regs_size[i];
    auto it = _regi_idx_map.find(ids[i]);
    assert(it != _regi_idx_map.end());
    auto &reg = _regi_arr[it->second];
    assert(reg_size == reg.size);
    reg.val.resize(reg_size);
    std::memcpy(&reg.val[0], in_bufs[i], reg_size);
  }
}

void DBClient::get_regs_infos(const vm_reg_t *ids, RegInfos *out_infos,
                              std::size_t nregs) {
  assert(_state == State::VM_STOPPED);
  _fetch_reg_infos_by_id(ids, nregs);

  for (std::size_t i = 0; i < nregs; ++i) {
    auto it = _regi_idx_map.find(ids[i]);
    assert(it != _regi_idx_map.end());
    out_infos[i] = _regi_arr[it->second];
  }
}

void DBClient::find_regs_ids(const char **reg_names, vm_reg_t *out_ids,
                             std::size_t nregs) {
  assert(_state == State::VM_STOPPED);
  _fetch_reg_infos_by_name(reg_names, nregs);

  for (std::size_t i = 0; i < nregs; ++i) {
    auto it = _regi_name_map.find(reg_names[i]);
    assert(it != _regi_name_map.end());
    out_ids[i] = _regi_arr[it->second].idx;
  }
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
                             std::vector<std::string> &out_text,
                             std::vector<vm_size_t> &out_sizes) {
  assert(_state == State::VM_STOPPED);
  _impl->get_code_text(addr, nins, out_text, out_sizes);
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

vm_reg_t DBClient::registers_count() const {
  assert(_state == State::VM_STOPPED);
  return _vm_infos.regs_count;
}

const std::vector<vm_reg_t> &DBClient::list_regs(RegKind kind) const {
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

vm_size_t DBClient::memory_size() const {
  assert(_state == State::VM_STOPPED);
  return _vm_infos.memory_size;
}

vm_sym_t DBClient::symbols_count() const {
  assert(_state == State::VM_STOPPED);
  return _vm_infos.symbols_count;
}

vm_size_t DBClient::pointer_size() const {
  assert(_state == State::VM_STOPPED);
  return _vm_infos.pointer_size;
}

vm_size_t DBClient::integer_size() const {
  assert(_state == State::VM_STOPPED);
  return _vm_infos.integer_size;
}

bool DBClient::use_opcode() const {
  assert(_state == State::VM_STOPPED);
  return _vm_infos.use_opcode;
}

// Caching

void DBClient::_discard_tmp_cache() {
  for (auto &inf : _regi_arr)
    inf.val.clear();
}

void DBClient::_fetch_reg_infos_by_id(const vm_reg_t *idxs, std::size_t nregs) {

  // Make list of all mising regs
  std::vector<vm_reg_t> miss;
  for (std::size_t i = 0; i < nregs; ++i)
    if (_regi_idx_map.find(idxs[i]) == _regi_idx_map.end())
      miss.push_back(idxs[i]);
  if (miss.empty())
    return;

  // Get missing regs
  nregs = miss.size();
  std::vector<RegInfos> infos(nregs);
  _impl->get_regs_infos(&miss[0], &infos[0], nregs);
  for (auto &inf : infos) // value not expected to be correct
    inf.val.clear();

  // Add to cache
  std::size_t new_idx = _regi_arr.size();
  _regi_arr.insert(_regi_arr.end(), infos.begin(), infos.end());
  for (std::size_t i = new_idx; i < _regi_arr.size(); ++i) {
    const auto &inf = _regi_arr[i];
    _regi_idx_map.emplace(inf.idx, i);
    _regi_name_map.emplace(inf.name, i);
  }
}

void DBClient::_fetch_reg_infos_by_name(const char **names, std::size_t nregs) {
  // Make list of all mising regs
  std::vector<const char *> miss;
  for (std::size_t i = 0; i < nregs; ++i)
    if (_regi_name_map.find(names[i]) == _regi_name_map.end())
      miss.push_back(names[i]);
  if (miss.empty()) {
    return;
  }

  // Load indices of missing regs
  nregs = miss.size();
  std::vector<vm_reg_t> idxs(nregs);
  _impl->find_regs_ids(&miss[0], &idxs[0], nregs);

  // Add to cache
  _fetch_reg_infos_by_id(&idxs[0], nregs);
}

void DBClient::_fetch_reg_vals_by_id(const vm_reg_t *idxs, std::size_t nregs) {
  _fetch_reg_infos_by_id(idxs, nregs);

  // Make list of all missing regs
  std::vector<vm_reg_t> miss;
  std::vector<vm_size_t> miss_size;
  vm_size_t total_size = 0;

  for (std::size_t i = 0; i < nregs; ++i) {
    auto it = _regi_idx_map.find(idxs[i]);
    assert(it != _regi_idx_map.end());
    auto &reg = _regi_arr[it->second];
    if (reg.val.empty()) {
      miss.push_back(reg.idx);
      miss_size.push_back(reg.size);
      total_size += reg.size;
    }
  }
  if (miss.empty())
    return;

  std::vector<char> full_buff(total_size);
  std::vector<char *> miss_bufs;
  char *buf_pos = &full_buff[0];
  for (std::size_t i = 0; i < miss.size(); ++i) {
    miss_bufs.push_back(buf_pos);
    buf_pos += miss_size[i];
  }

  // Load data
  nregs = miss.size();
  _impl->get_regs(&miss[0], &miss_bufs[0], &miss_size[0], nregs);

  // Add data to cache
  buf_pos = &full_buff[0];
  for (std::size_t i = 0; i < nregs; ++i) {
    auto it = _regi_idx_map.find(miss[i]);
    assert(it != _regi_idx_map.end());
    auto &reg = _regi_arr[it->second];
    reg.val.resize(miss_size[i]);
    std::memcpy(&reg.val[0], buf_pos, miss_size[i]);
    buf_pos += miss_size[i];
  }
}

} // namespace odb
