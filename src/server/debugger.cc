#include "odb/server/debugger.hh"

#include <algorithm>
#include <cassert>

#include <iostream>

namespace odb {

namespace {

constexpr vm_ptr_t SYM_LOAD_SIZE = 256;

}

Debugger::Debugger(std::unique_ptr<VMApi> &&vm)
    : _vm(std::move(vm)), _state(State::NOT_STARTED) {
  assert(_vm.get());
}

void Debugger::on_init() {
  assert(_state == State::NOT_STARTED);
  // 1) get general VM infos
  _state = State::RUNNING_TOFINISH;
  _infos = _vm->get_vm_infos();
  _syms_ranges = std::make_unique<RangeMap<int>>(0, _infos.memory_size - 1, 0);

  // 2) Get extra usefull informations
  // @EXTRA get all registers if count below a threshold
  // @EXTRA get all symbols if count below a threshold

  // 3) Get entry point and init stack frame
  auto upd = _vm->get_update_infos();
  assert(upd.state == VMApi::UpdateState::OK);
  _ins_addr = upd.act_addr;

  CallInfos start;
  start.caller_start_addr = _ins_addr;
  _call_stack.push_back(start);
}

void Debugger::on_update() {
  assert(_state != State::NOT_STARTED && _state != State::STOPPED &&
         _state != State::ERROR && _state != State::EXIT);

  auto udp = _vm->get_update_infos();
  if (udp.state == VMApi::UpdateState::ERROR) {
    _state = State::ERROR;
    return;
  }
  if (udp.state == VMApi::UpdateState::EXIT) {
    _state = State::EXIT;
    return;
  }

  auto old_addr = _ins_addr;
  _ins_addr = udp.act_addr;

  if (udp.state == VMApi::UpdateState::CALL_SUB) {
    _call_stack.back().call_addr = old_addr;
    CallInfos new_call;
    new_call.caller_start_addr = _ins_addr;
    _call_stack.push_back(new_call);
  } else if (udp.state == VMApi::UpdateState::RET_SUB) {
    assert(!_call_stack.empty());
    _call_stack.pop_back();
  }

  if (_state == State::RUNNING_TOFINISH) {
    return;
  }

  if (_state == State::RUNNING_STEP) {
    _state = State::STOPPED;
    return;
  }

  if (_state == State::RUNNING_STEP_OVER &&
      _step_over_depth == _call_stack.size()) {
    _state = State::STOPPED;
    return;
  }

  if (_state == State::RUNNING_STEP_OUT &&
      udp.state == VMApi::UpdateState::RET_SUB) {
    _state = State::STOPPED;
    return;
  }

  if (_breakpts.find(_ins_addr) != _breakpts.end())
    _state = State::STOPPED;
}

void Debugger::get_reg(vm_reg_t idx, std::uint8_t *val) {
  // @EXTRA add caching system to avoid reloading the register every time
  _load_reg(idx);
  auto &infos = _map_regs.find(idx)->second;
  _vm->get_reg(idx, infos, true);
  std::copy_n(&infos.val[0], infos.size, val);
}

void Debugger::set_reg(vm_reg_t idx, const std::uint8_t *new_val) {
  _vm->set_reg(idx, new_val);
}

RegInfos Debugger::get_reg_infos(vm_reg_t idx) {
  _load_reg(idx);
  return _map_regs.find(idx)->second;
}

vm_reg_t Debugger::find_reg_id(const std::string &name) {
  auto it = _smap_regs.find(name);
  if (it != _smap_regs.end())
    return it->second;

  auto id = _vm->find_reg_id(name);
  _load_reg(id);
  return id;
}

vm_reg_t Debugger::registers_count() { return _infos.regs_count; }

const std::vector<vm_reg_t> &Debugger::list_regs(RegKind kind) const {
  if (kind == RegKind::general)
    return _infos.regs_general;
  else if (kind == RegKind::program_counter)
    return _infos.regs_program_counter;
  else if (kind == RegKind::stack_pointer)
    return _infos.regs_stack_pointer;
  else if (kind == RegKind::base_pointer)
    return _infos.regs_base_pointer;
  else if (kind == RegKind::flags)
    return _infos.regs_flags;
  else
    throw VMApi::Error("list_regs: invalid reg kind");
}

void Debugger::read_mem(vm_ptr_t addr, vm_size_t size, std::uint8_t *out_buf) {
  _vm->read_mem(addr, size, out_buf);
}

void Debugger::write_mem(vm_ptr_t addr, vm_size_t size,
                         const std::uint8_t *buf) {
  _vm->write_mem(addr, size, buf);
}

vm_size_t Debugger::get_memory_size() { return _infos.memory_size; }

vm_sym_t Debugger::get_symbol_at(vm_ptr_t addr) {
  _preload_symbols(addr);
  auto it = _syms_pos.find(addr);
  return it == _syms_pos.end() ? VM_SYM_NULL : it->second;
}

std::vector<vm_sym_t> Debugger::get_symbols(vm_ptr_t addr, vm_size_t size) {
  _preload_symbols(addr, size);
  auto end = addr + size;
  std::vector<vm_sym_t> res;

  auto it = _syms_pos.lower_bound(addr);
  while (it != _syms_pos.end() && it->first < end) {
    res.push_back(it->second);
    ++it;
  }

  return res;
}

SymbolInfos Debugger::get_symbol_infos(vm_sym_t idx) {
  _load_symbol(idx);
  auto it = _map_syms.find(idx);
  assert(it != _map_syms.end());
  return it->second;
}

vm_sym_t Debugger::symbols_count() { return _infos.symbols_count; }

vm_sym_t Debugger::find_sym_id(const std::string &name) {
  auto id = _vm->find_sym_id(name);
  _load_symbol(id);
  return id;
}

std::string Debugger::get_code_text(vm_ptr_t addr, vm_size_t &addr_dist) {
  // @EXTRA: cache result and/or be able to load more than one at once
  return _vm->get_code_text(addr, addr_dist);
}

vm_ptr_t Debugger::get_execution_point() { return _ins_addr; }

void Debugger::add_breakpoint(vm_ptr_t addr) {
  if (addr >= _infos.memory_size)
    throw VMApi::Error(
        "cannot add breakpoint: address outside of memory range");
  if (!_breakpts.insert(addr).second)
    throw VMApi::Error(
        "cannot add breakpoint: There is already one at this address");
}

bool Debugger::has_breakpoint(vm_ptr_t addr) {
  if (addr >= _infos.memory_size)
    throw VMApi::Error(
        "cannot get breakpoint: address outside of memory range");
  return _breakpts.find(addr) != _breakpts.end();
}

void Debugger::del_breadkpoint(vm_ptr_t addr) {
  if (addr >= _infos.memory_size)
    throw VMApi::Error(
        "cannot delete breakpoint: address outside of memory range");
  if (_breakpts.erase(addr) == 0)
    throw VMApi::Error(
        "cannot delete breakpoint: there is none at this address");
}

void Debugger::resume(ResumeType type) {
  if (_state == State::EXIT || _state == State::ERROR)
    throw VMApi::Error("cannot resume execution: program already finished");

  if (type == ResumeType::ToFinish)
    _state = State::RUNNING_TOFINISH;
  else if (type == ResumeType::Continue)
    _state = State::RUNNING_BKP;
  else if (type == ResumeType::Step) {
    _state = State::RUNNING_STEP;
  } else if (type == ResumeType::StepOver) {
    _state = State::RUNNING_STEP_OVER;
    _step_over_depth = _call_stack.size();
  } else if (type == ResumeType::StepOut)
    _state = State::RUNNING_STEP_OUT;
}

void Debugger::stop() {
  assert(_state != State::NOT_STARTED);
  if (_state == State::EXIT || _state == State::ERROR)
    throw VMApi::Error("cannot stop execution: program already finished");
  if (_state == State::STOPPED)
    throw VMApi::Error("cannot stop execution: program already stopped");
}

void Debugger::_load_reg(vm_reg_t id) {

  if (_map_regs.find(id) != _map_regs.end())
    return;

  RegInfos infos;
  _vm->get_reg(id, infos, false);
  infos.val.resize(infos.size);
  _map_regs.emplace(id, infos);
  _smap_regs.emplace(infos.name, id);
}

void Debugger::_preload_symbols(vm_ptr_t addr, vm_size_t size) {
  size = std::max(size, SYM_LOAD_SIZE);
  auto end = addr + size - 1;
  end = std::min(end, _infos.memory_size - 1);

  auto low_range = _syms_ranges->range_of(addr);
  if (low_range.val == 1) {
    addr = low_range.high + 1;
    if (!addr || addr > end) // everything already loaded
      return;
  }

  auto high_range = _syms_ranges->range_of(end);
  if (high_range.val == 1) {
    end = high_range.low - 1;
    if (!high_range.low || end < addr)
      return;
  }

  size = end - addr + 1;
  auto syms = _vm->get_symbols(addr, size);
  _syms_ranges->set(addr, end, 1);
  for (const auto &s : syms)
    _load_symbol(s);
}

void Debugger::_preload_symbols(vm_ptr_t addr) {
  auto size = SYM_LOAD_SIZE;
  addr = addr < size / 2 ? 0 : addr - size / 2;
  _preload_symbols(addr, size);
}

void Debugger::_load_symbol(vm_sym_t id) {
  if (_map_syms.find(id) != _map_syms.end())
    return;

  auto infos = _vm->get_symb_infos(id);
  _map_syms.emplace(id, infos);
  _smap_syms.emplace(infos.name, id);
  _syms_pos.emplace(infos.addr, id);
}

} // namespace odb
