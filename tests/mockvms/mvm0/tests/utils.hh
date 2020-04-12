#include <catch2/catch.hpp>

#include "../include/mvm0/cpu.hh"
#include "../include/mvm0/parser.hh"
#include "../include/mvm0/rom.hh"
#include "../include/mvm0/vm-api.hh"

#include <odb/server/debugger.hh>

inline std::uint32_t db_get_reg(odb::Debugger &db, odb::vm_reg_t idx) {
  std::uint32_t res;
  db.get_reg(idx, reinterpret_cast<std::uint8_t *>(&res));
  return res;
}

inline void db_set_reg(odb::Debugger &db, odb::vm_reg_t idx,
                       std::uint32_t val) {
  db.set_reg(idx, reinterpret_cast<std::uint8_t *>(&val));
}

inline std::uint32_t db_read_u32(odb::Debugger &db, std::size_t addr) {
  std::uint32_t res;
  db.read_mem(addr, 4, reinterpret_cast<std::uint8_t *>(&res));
  return res;
}

inline void db_write_u32(odb::Debugger &db, std::size_t addr,
                         std::uint32_t val) {
  db.write_mem(addr, 4, reinterpret_cast<std::uint8_t *>(&val));
}

inline void db_resume(odb::Debugger &db, mvm0::CPU &cpu, odb::ResumeType type) {
  db.resume(type);
  while (db.get_state() != odb::Debugger::State::EXIT &&
         db.get_state() != odb::Debugger::State::ERROR &&
         db.get_state() != odb::Debugger::State::STOPPED) {
    cpu.step();
    db.on_update();
  }
}
