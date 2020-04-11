#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "defs.hh"
#include "rom.hh"

namespace mvm0 {

class VMApi;

class CPU {
public:
  CPU(const ROM &rom);

  enum class Status {
    OK,
    NORMAL_EXIT,
    BAD_INS,
    SEGV,
  };

  // Make CPU ready for execution
  void init();

  // Run the next instruction
  // Returns 0 if CPU ready for next instruction, 1 if stopped
  // Call status() for more infos if stopped
  int step();

  Status status() const { return _status; }

private:
  std::array<std::uint32_t, BASE_REGS> _regs;
  std::uint32_t _pc;
  std::uint32_t _prev_pc;
  std::uint32_t _zf;
  std::vector<std::uint8_t> _ram;
  std::uint32_t _guard; // returned memory access when SEGV
  ROM _rom;
  Status _status;

  std::uint32_t &_reg(int idx);
  std::uint32_t &_mem(int addr);

  friend class VMApi;
};

} // namespace mvm0
