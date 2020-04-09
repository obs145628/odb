#include "odb/server/debugger.hh"

#include <cassert>

namespace odb {

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
  start.caller_sym = 0; //@TODO compute val
  _call_stack.push_back(start);
}

} // namespace odb
