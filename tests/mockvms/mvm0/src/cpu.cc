#include "../include/mvm0/cpu.hh"

#include <cassert>
#include <cstring>

namespace {

std::uint32_t imm(int x) {
  assert(x >= 0);
  return static_cast<std::uint32_t>(x);
}

} // namespace

namespace mvm0 {

CPU::CPU(const ROM &rom)
    : _ram(MEM_RAM_SIZE, 0), _rom(rom), _status(Status::OK) {}

void CPU::init() {
  std::fill(_regs.begin(), _regs.end(), 0);
  _regs[REG_SP] = 1024;
  _pc = 1024;
  _prev_pc = 0;
  _zf = 0;
}

// Run the next instruction
// Returns 0 if CPU ready for next instruction, 1 if stopped
// Call status() for more infos if stopped
int CPU::step() {
  if (_pc < MEM_CODE_START || _pc >= MEM_SIZE) {
    _status = Status::SEGV;
    return 1;
  }

  const auto &ins = _rom.ins[_pc - MEM_CODE_START];
  auto a0 = ins.args.size() > 0 ? ins.args[0] : 0;
  auto a1 = ins.args.size() > 1 ? ins.args[1] : 0;
  auto a2 = ins.args.size() > 2 ? ins.args[2] : 0;
  auto pc_cpy = _pc;

  if (ins.name == "mov") {
    _reg(a1) = _reg(a0);
    ++_pc;
  } else if (ins.name == "movi") {
    _reg(a1) = imm(a0);
    ++_pc;
  } else if (ins.name == "ldr") {
    _reg(a1) = _mem(_reg(a0));
    ++_pc;
  } else if (ins.name == "str") {
    _mem(_reg(a1)) = _reg(a0);
    ++_pc;
  } else if (ins.name == "b") {
    _pc = imm(a0);
  } else if (ins.name == "bz") {
    if (_zf)
      _pc = imm(a0);
    else
      ++_pc;
  } else if (ins.name == "bn") {
    if (!_zf)
      _pc = imm(a0);
    else
      ++_pc;
  } else if (ins.name == "call") {
    _regs[REG_SP] -= 4;
    _mem(_regs[REG_SP]) = _pc + 1;
    _pc = imm(a0);
  } else if (ins.name == "ret") {
    _pc = _mem(_regs[REG_SP]);
    _regs[REG_SP] += 4;
  } else if (ins.name == "sys") {
    std::size_t code = imm(a0);
    if (code == 0)
      _status = Status::NORMAL_EXIT;
    else
      _status = Status::BAD_INS;
    ++_pc;
  } else if (ins.name == "add") {
    _reg(a2) = _reg(a0) + _reg(a1);
    ++_pc;
  } else if (ins.name == "sub") {
    _reg(a2) = _reg(a0) - _reg(a1);
    ++_pc;
  } else if (ins.name == "mul") {
    _reg(a2) = _reg(a0) * _reg(a1);
    ++_pc;
  } else if (ins.name == "div") {
    _reg(a2) = _reg(a0) / _reg(a1);
    ++_pc;
  } else if (ins.name == "mod") {
    _reg(a2) = _reg(a0) % _reg(a1);
    ++_pc;
  } else if (ins.name == "and") {
    _reg(a2) = _reg(a0) & _reg(a1);
    ++_pc;
  } else if (ins.name == "or") {
    _reg(a2) = _reg(a0) | _reg(a1);
    ++_pc;
  } else if (ins.name == "xor") {
    _reg(a2) = _reg(a0) ^ _reg(a1);
    ++_pc;
  } else if (ins.name == "shl") {
    _reg(a2) = _reg(a0) >> _reg(a1);
    ++_pc;
  } else if (ins.name == "shr") {
    _reg(a2) = _reg(a0) << _reg(a1);
    ++_pc;
  }

  else {
    _status = Status::BAD_INS;
  }

  if (_status == Status::OK)
    _prev_pc = pc_cpy;

  return _status != Status::OK;
}

void CPU::read_ram(std::size_t addr, std::size_t size, void *out_buf) const {
  assert(addr < MEM_CODE_START);
  assert(addr + size <= MEM_CODE_START);
  const std::uint8_t *ptr = &_ram[0] + addr;
  std::memcpy(out_buf, ptr, size);
}

std::uint32_t &CPU::_reg(int idx) {
  auto sidx = static_cast<std::size_t>(idx);
  assert(sidx < BASE_REGS);
  return _regs[sidx];
}

std::uint32_t &CPU::_mem(int addr) {
  auto sptr = static_cast<std::size_t>(addr);
  if (sptr >= MEM_CODE_START) {
    _status = Status::SEGV;
    return _guard;
  }

  std::uint8_t *ptr = &_ram[0] + addr;
  return *(reinterpret_cast<std::uint32_t *>(ptr));
}

} // namespace mvm0
