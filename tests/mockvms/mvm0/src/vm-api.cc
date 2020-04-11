#include "../include/mvm0/vm-api.hh"

#include "../include/mvm0/cpu.hh"
#include "../include/mvm0/defs.hh"

#include <cassert>
#include <cstring>
#include <sstream>

namespace mvm0 {

namespace {
constexpr odb::vm_reg_t REG_PC = 16;
constexpr odb::vm_reg_t REG_ZF = 17;
constexpr odb::vm_reg_t NB_REGS = 18;
constexpr odb::vm_reg_t REG_SIZE = 4;

constexpr const char *reg_names[] = {"r0",  "r1",  "r2",  "r3", "r4",  "r5",
                                     "r6",  "r7",  "r8",  "r9", "r10", "r11",
                                     "r12", "r13", "r14", "sp", "pc",  "zf"};

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
    auto ptr = _reg_ptr(idx);
    assert(infos.val.size() == REG_SIZE);
    std::memcpy(&infos.val[0], ptr, REG_SIZE);
  }

  else {
    infos.idx = idx;
    infos.name = reg_names[idx];
    if (idx < REG_PC) {
      infos.kind = odb::RegKind::general;
    } else if (idx == REG_PC) {
      infos.kind = odb::RegKind::program_counter;
    } else if (idx == REG_ZF) {
      infos.kind = odb::RegKind::flags;
    }
    infos.size = REG_SIZE;
  }
}

void VMApi::set_reg(odb::vm_reg_t idx, const std::uint8_t *new_val) {
  if (idx >= NB_REGS)
    throw odb::VMApi::Error("invalid register index");
  auto ptr = _reg_ptr(idx);
  std::memcpy(ptr, new_val, REG_SIZE);
}

odb::vm_reg_t VMApi::find_reg_id(const std::string &name) {

  if (name.size() == 2 && name[0] == 'r') {
    char c1 = name[1];
    if (c1 < '0' && c1 > '9')
      throw odb::VMApi::Error("Invalid register name");
    return c1 - '0';
  }

  if (name.size() == 3 && name[0] == 'r') {
    char c1 = name[1];
    char c2 = name[2];
    if (c1 < '0' || c1 > '9' || c2 < '0' || c2 > '4')
      throw odb::VMApi::Error("Invalid register name");
    return (c1 - '0') * 10 + c2;
  }

  if (name == "sp")
    return REG_SP;
  if (name == "pc")
    return REG_PC;
  if (name == "zf")
    return REG_ZF;

  throw odb::VMApi::Error("Invalid register name");
}

void VMApi::read_mem(odb::vm_ptr_t addr, odb::vm_size_t size,
                     std::uint8_t *out_buf) {
  if (addr >= MEM_SIZE || addr + size > MEM_SIZE)
    throw odb::VMApi::Error("Memory address out of range (0-2048)");

  for (auto it = addr; it < addr + size; ++it) {
    std::uint8_t val = it < MEM_ROM_SIZE ? _cpu._ram[it] : 0;
    *(out_buf++) = val;
  }
}

void VMApi::write_mem(odb::vm_ptr_t addr, odb::vm_size_t size,
                      const std::uint8_t *buf) {
  if (addr >= MEM_SIZE || addr + size > MEM_SIZE)
    throw odb::VMApi::Error("Memory address out of range (0-2048)");

  for (auto it = addr; it < addr + size; ++it) {
    if (it < MEM_ROM_SIZE)
      _cpu._ram[it] = *buf;
    ++buf;
  }
}

std::vector<odb::vm_sym_t> VMApi::get_symbols(odb::vm_ptr_t addr,
                                              odb::vm_size_t size) {
  if (addr >= MEM_SIZE || addr + size > MEM_SIZE)
    throw odb::VMApi::Error("Memory address out of range (0-2048)");
  auto end = addr + size;
  addr = std::min(addr, MEM_CODE_START);
  std::vector<odb::vm_sym_t> res;

  for (auto it = addr; it < end; ++it) {
    const auto &ins = _cpu._rom.ins[it - MEM_CODE_START];
    if (ins.def_sym != SYM_NONE)
      res.push_back(ins.def_sym);
  }

  return res;
}

odb::SymbolInfos VMApi::get_symb_infos(odb::vm_sym_t idx) {
  const auto &rom = _cpu._rom;
  if (idx >= rom.syms.size())
    throw odb::VMApi::Error("Invalid symbol index");

  odb::SymbolInfos res;
  res.idx = idx;
  res.name = rom.syms[idx];
  res.addr = MEM_CODE_START + rom.sym_defs[idx];
  return res;
}

odb::vm_sym_t VMApi::find_sym_id(const std::string &name) {
  const auto &rom = _cpu._rom;
  auto it = rom.smap.find(name);
  if (it == rom.smap.end())
    throw odb::VMApi::Error("Invalid symbol name");
  return it->second;
}

std::string VMApi::get_code_text(odb::vm_ptr_t addr,
                                 odb::vm_size_t &addr_dist) {
  if (addr >= MEM_SIZE)
    throw odb::VMApi::Error("Memory address out of range (0-2048)");
  addr_dist = 1;
  if (addr < MEM_CODE_START)
    return "";

  const auto &ins = _cpu._rom.ins[addr - MEM_CODE_START];
  std::ostringstream os;

  if (ins.name == "movi") {
    os << "movi " << reg_names[ins.args[0]] << " ";
    if (ins.use_sym)
      os << "{" << ins.use_sym << "}";
    else
      os << ins.args[1];
  }

  else if (ins.name == "b") {
    os << "b ";
    if (ins.use_sym)
      os << "{" << ins.use_sym << "}";
    else
      os << ins.args[0];
  }

  else if (ins.name == "bz") {
    os << "bz ";
    if (ins.use_sym)
      os << _cpu._rom.syms[ins.use_sym];
    else
      os << ins.args[0];
  }

  else if (ins.name == "bn") {
    os << "bn ";
    if (ins.use_sym)
      os << "{" << ins.use_sym << "}";
    else
      os << ins.args[0];
  }

  else if (ins.name == "call") {
    os << "call ";
    if (ins.use_sym)
      os << "{" << ins.use_sym << "}";
    else
      os << ins.args[0];
  }

  else if (ins.name == "sys") {
    os << "sys " << ins.args[0];
  }

  else {
    os << ins.name;
    for (const auto &arg : ins.args)
      os << ' ' << reg_names[arg];
  }

  return os.str();
}

std::uint32_t *VMApi::_reg_ptr(odb::vm_reg_t idx) {
  std::uint32_t *reg_ptr;
  if (idx < REG_PC)
    reg_ptr = &_cpu._regs[idx];
  else if (idx == REG_PC)
    reg_ptr = &_cpu._pc;
  else if (idx == REG_ZF)
    reg_ptr = &_cpu._zf;

  return reg_ptr;
}

} // namespace mvm0
