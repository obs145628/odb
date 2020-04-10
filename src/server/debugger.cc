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
  // 1) get general VM infos
  _state = State::RUNNING;
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
  auto udp = _vm->get_update_infos();
  if (udp.state == VMApi::UpdateState::ERROR)
    _state = State::ERROR;
  else if (udp.state == VMApi::UpdateState::EXIT)
    _state = State::EXIT;
  // @TODO other states
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
