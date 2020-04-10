#include "odb/server/debugger.hh"

#include <cassert>

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
  // @TODO get all registers if count below a threshold
  // @TODO get all symbols if count below a threshold

  // 3) Get entry point and init stack frame
  auto upd = _vm->get_update_infos();
  assert(upd.state == VMApi::UpdateState::OK);
  _ins_addr = upd.act_addr;

  CallInfos start;
  start.caller_start_addr = _ins_addr;
  _call_stack.push_back(start);
}

void Debugger::on_update() {
  assert(_state == State::NOT_STARTED && _state != State::STOPPED &&
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
  else if (type == ResumeType::Step)
    _state = State::RUNNING_STEP;
  else if (type == ResumeType::StepOver)
    _state = State::RUNNING_STEP_OVER;
  else if (type == ResumeType::StepOut)
    _state = State::RUNNING_STEP_OUT;
}

void Debugger::stop() {
  assert(_state != State::NOT_STARTED);
  if (_state == State::EXIT || _state == State::ERROR)
    throw VMApi::Error("cannot stop execution: program already finished");
  if (_state == State::STOPPED)
    throw VMApi::Error("cannot stop execution: program already stopped");
}

} // namespace odb
