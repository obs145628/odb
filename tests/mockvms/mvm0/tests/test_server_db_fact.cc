#include "utils.hh"

#define PATH_CALL_FACT (MVM0_EXS_DIR + std::string("call_fact.vv"))

namespace {}

TEST_CASE("debug call_fact exec", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_FACT);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();
  db_resume(db, cpu, odb::ResumeType::Continue);

  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1046);
  REQUIRE(db.get_code_text(1046, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 10) == 24);
}

TEST_CASE("debug call_fact check_stack", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_FACT);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();

  db.add_breakpoint(1043);
  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_execution_point() == 1043);
  REQUIRE(db.get_code_text(1043, un) == "call {1}");
  REQUIRE(db_get_reg(db, 15) == 1024);
  auto cs = db.get_call_stack();
  REQUIRE(cs.size() == 1);
  REQUIRE(cs[0].caller_start_addr == 1024);

  db.add_breakpoint(1041);
  db_resume(db, cpu, odb::ResumeType::Continue);
  db.del_breakpoint(1041);
  REQUIRE(db.get_execution_point() == 1041);
  REQUIRE(db.get_code_text(1041, un) == "ret");

  // check vm stack
  REQUIRE(db_get_reg(db, 15) == 996); // sp
  for (std::size_t i = 0; i < 3; ++i) {
    REQUIRE(db_read_u32(db, 996 + 8 * i) == 1036);
    REQUIRE(db_read_u32(db, 1000 + 8 * i) == i + 2);
  }
  REQUIRE(db_read_u32(db, 1020) == 1044);

  // check db stack
  cs = db.get_call_stack();
  REQUIRE(cs.size() == 5);
  REQUIRE(cs[4].caller_start_addr == 1025);
  for (std::size_t i = 1; i < 4; ++i) {
    REQUIRE(cs[i].caller_start_addr == 1025);
    REQUIRE(cs[i].call_addr == 1035);
  }
  REQUIRE(cs[0].caller_start_addr == 1024);
  REQUIRE(cs[0].call_addr == 1043);

  db.add_breakpoint(1044);
  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_execution_point() == 1044);
  REQUIRE(db.get_code_text(1044, un) == "mov r0 r10");
  REQUIRE(db_get_reg(db, 0) == 24);
  REQUIRE(db_get_reg(db, 15) == 1024);
  cs = db.get_call_stack();
  REQUIRE(cs.size() == 1);
  REQUIRE(cs[0].caller_start_addr == 1024);

  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1046);
  REQUIRE(db.get_code_text(1046, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 10) == 24);
}

TEST_CASE("debug call_fact infos", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_FACT);
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();
  REQUIRE(db.get_state() == odb::Debugger::State::RUNNING_TOFINISH);

  REQUIRE(db.get_symbols(1024, 23) ==
          std::vector<odb::vm_sym_t>{0, 1, 2, 3, 4, 5});

  auto sym0 = db.get_symbol_infos(0);
  REQUIRE(sym0.idx == 0);
  REQUIRE(sym0.name == "_begin");
  REQUIRE(sym0.addr == 1024);

  auto sym1 = db.get_symbol_infos(1);
  REQUIRE(sym1.idx == 1);
  REQUIRE(sym1.name == "fact");
  REQUIRE(sym1.addr == 1025);

  auto sym2 = db.get_symbol_infos(2);
  REQUIRE(sym2.idx == 2);
  REQUIRE(sym2.name == "fact_base");
  REQUIRE(sym2.addr == 1030);

  auto sym3 = db.get_symbol_infos(3);
  REQUIRE(sym3.idx == 3);
  REQUIRE(sym3.name == "fact_rec");
  REQUIRE(sym3.addr == 1032);

  auto sym4 = db.get_symbol_infos(4);
  REQUIRE(sym4.idx == 4);
  REQUIRE(sym4.name == "fact_end");
  REQUIRE(sym4.addr == 1041);

  auto sym5 = db.get_symbol_infos(5);
  REQUIRE(sym5.idx == 5);
  REQUIRE(sym5.name == "_start");
  REQUIRE(sym5.addr == 1042);

  REQUIRE(db.get_symbol_at(1024) == 0);
  REQUIRE(db.get_symbol_at(1025) == 1);
  REQUIRE(db.get_symbol_at(1026) == odb::VM_SYM_NULL);
  REQUIRE(db.get_symbol_at(1032) == 3);
  REQUIRE(db.get_symbol_at(1041) == 4);
  REQUIRE(db.get_symbol_at(1042) == 5);
  REQUIRE(db.symbols_count() == 6);

  REQUIRE(db.find_sym_id("_begin") == 0);
  REQUIRE(db.find_sym_id("fact") == 1);
  REQUIRE(db.find_sym_id("fact_base") == 2);
  REQUIRE(db.find_sym_id("fact_rec") == 3);
  REQUIRE(db.find_sym_id("fact_end") == 4);
  REQUIRE(db.find_sym_id("_start") == 5);

  odb::vm_size_t un;
  REQUIRE(db.get_code_text(1023, un) == "");
  REQUIRE(db.get_code_text(1024, un) == "b {5}");
  REQUIRE(db.get_code_text(1028, un) == "bz {2}");
  REQUIRE(db.get_code_text(1029, un) == "b {3}");
  REQUIRE(db.get_code_text(1031, un) == "b {4}");
  REQUIRE(db.get_code_text(1035, un) == "call {1}");
  REQUIRE(db.get_code_text(1040, un) == "b {4}");
  REQUIRE(db.get_code_text(1043, un) == "call {1}");
  REQUIRE(db.get_code_text(1046, un) == "sys 0");
  REQUIRE(db.get_code_text(1047, un) == "");

  REQUIRE(db.get_execution_point() == 1024);
}
