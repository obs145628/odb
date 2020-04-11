#include "../include/mvm0/vm-api.hh"

#include "../include/mvm0/cpu.hh"
#include "../include/mvm0/defs.hh"

#include <cassert>
#include <cstring>

namespace mvm0 {

namespace {
constexpr odb::vm_reg_t REG_PC = 16;
constexpr odb::vm_reg_t REG_ZF = 17;
constexpr odb::vm_reg_t NB_REGS = 18;
constexpr odb::vm_reg_t REG_SIZE = 4;
} // namespace

VMApi::VMApi(CPU &cpu) : _cpu(cpu) {}

odb::VMInfos VMApi::get_vm_infos() {
  odb::VMInfos infos;
  infos.name = "mvm0";
  infos.regs_count = NB_REGS;
  infos.regs_general = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
  infos.regs_program_counter = {REG_PC};
  infos.regs_stack_pointer = {REG_SP};
  infos.regs_flags = {REG_ZF};
  infos.memory_size = MEM_SIZE;

  return infos;
}

odb::VMApi::UpdateInfos VMApi::get_update_infos() {
  odb::VMApi::UpdateInfos infos;
  infos.act_addr = _cpu._pc;

  if (_cpu._prev_pc == 0) {
    infos.state = odb::VMApi::UpdateState::OK;
    return infos;
  }

  if (_cpu.status() == CPU::Status::BAD_INS ||
      _cpu.status() == CPU::Status::SEGV) {
    infos.state = odb::VMApi::UpdateState::ERROR;
    return infos;
  }

  if (_cpu.status() == CPU::Status::NORMAL_EXIT) {
    infos.state = odb::VMApi::UpdateState::EXIT;
    return infos;
  }

  auto &prev_ins = _cpu._rom.ins[_cpu._prev_pc - MEM_CODE_START];
  if (prev_ins.name == "call")
    infos.state = odb::VMApi::UpdateState::CALL_SUB;
  else if (prev_ins.name == "ret")
    infos.state = odb::VMApi::UpdateState::RET_SUB;
  else
    infos.state = odb::VMApi::UpdateState::OK;

  return infos;
}

void VMApi::get_reg(odb::vm_reg_t idx, odb::RegInfos &infos, bool val_only) {
  if (idx >= NB_REGS)
    throw odb::VMApi::Error("invalid register index");

  if (val_only) {
    std::uint32_t *reg_ptr;
    if (idx < REG_PC)
      reg_ptr = &_cpu._regs[idx];
    else if (idx == REG_PC)
      reg_ptr = &_cpu._pc;
    else if (idx == REG_ZF)
      reg_ptr = &_cpu._zf;

    assert(infos.val.size() == REG_SIZE);
    std::memcpy(&infos.val[0], reg_ptr, REG_SIZE);
  }

  else {
    infos.idx = idx;
    if (idx < REG_PC) {
      infos.name = "r" + std::to_string(idx);
      infos.kind = odb::RegKind::general;
    } else if (idx == REG_PC) {
      infos.name = "pc";
      infos.kind = odb::RegKind::program_counter;
    } else if (idx == REG_ZF) {
      infos.name = "zf";
      infos.kind = odb::RegKind::flags;
    }
    infos.size = REG_SIZE;
  }
}

#if 0
  
  void set_reg(odb::vm_reg_t idx, const std::uint8_t *new_val) override;
odb::vm_reg_t find_reg_id(const std::string &name) override;

void read_mem(odb::vm_ptr_t addr, odb::vm_size_t size,
              std::uint8_t *out_buf) override;
void write_mem(odb::vm_ptr_t addr, odb::vm_size_t size,
               const std::uint8_t *buf) override;

std::vector<odb::vm_sym_t> get_symbols(odb::vm_ptr_t addr,
                                       odb::vm_size_t size) override;
odb::SymbolInfos get_symb_infos(odb::vm_sym_t idx) override;
odb::vm_sym_t find_sym_id(const std::string &name) override;

std::string get_code_text(odb::vm_ptr_t addr,
                          odb::vm_size_t &addr_dist) override;
#endif
} // namespace mvm0
