#include "utils.hh"

namespace {

std::uint32_t read_u32(const mvm0::CPU &cpu, std::size_t addr) {
  std::uint32_t res;
  cpu.read_ram(addr, 4, &res);
  return res;
}

void db_step(odb::Debugger &db, mvm0::CPU &cpu) {
  db.resume(odb::ResumeType::Step);
  REQUIRE(cpu.step() == 0);
  db.on_update();
  REQUIRE(db.get_state() == odb::Debugger::State::STOPPED);
}

} // namespace

TEST_CASE("rom call_add", "") {
  auto rom = mvm0::parse_file(PATH_CALL_ADD);
  REQUIRE(rom.ins.size() == 9);

  REQUIRE(rom.sym_defs == std::vector<std::size_t>{0, 1, 3});
  REQUIRE(rom.syms == std::vector<std::string>{"_begin", "my_add", "_start"});
  REQUIRE(rom.smap.size() == 3);
  REQUIRE(rom.smap["_begin"] == 0);
  REQUIRE(rom.smap["my_add"] == 1);
  REQUIRE(rom.smap["_start"] == 2);

  REQUIRE(rom.ins[0].name == "b");
  REQUIRE(rom.ins[0].args.size() == 1);
  REQUIRE(rom.ins[0].def_sym == 0);
  REQUIRE(rom.ins[0].use_sym == 2);

  REQUIRE(rom.ins[1].name == "add");
  REQUIRE(rom.ins[1].def_sym == 1);
  REQUIRE(rom.ins[1].use_sym == mvm0::SYM_NONE);
  REQUIRE(rom.ins[1].args == std::vector<int>{0, 1, 0});

  REQUIRE(rom.ins[2].name == "ret");
  REQUIRE(rom.ins[2].args.size() == 0);
  REQUIRE(rom.ins[2].def_sym == mvm0::SYM_NONE);
  REQUIRE(rom.ins[2].use_sym == mvm0::SYM_NONE);

  REQUIRE(rom.ins[3].name == "movi");
  REQUIRE(rom.ins[3].def_sym == 2);
  REQUIRE(rom.ins[3].use_sym == mvm0::SYM_NONE);
  REQUIRE(rom.ins[3].args == std::vector<int>{12, 0});

  REQUIRE(rom.ins[4].name == "movi");
  REQUIRE(rom.ins[4].def_sym == mvm0::SYM_NONE);
  REQUIRE(rom.ins[4].use_sym == mvm0::SYM_NONE);
  REQUIRE(rom.ins[4].args == std::vector<int>{45, 1});

  REQUIRE(rom.ins[5].name == "call");
  REQUIRE(rom.ins[5].args.size() == 1);
  REQUIRE(rom.ins[5].def_sym == mvm0::SYM_NONE);
  REQUIRE(rom.ins[5].use_sym == 1);

  REQUIRE(rom.ins[6].name == "mov");
  REQUIRE(rom.ins[6].def_sym == mvm0::SYM_NONE);
  REQUIRE(rom.ins[6].use_sym == mvm0::SYM_NONE);
  REQUIRE(rom.ins[6].args == std::vector<int>{0, 10});

  REQUIRE(rom.ins[7].name == "movi");
  REQUIRE(rom.ins[7].def_sym == mvm0::SYM_NONE);
  REQUIRE(rom.ins[7].use_sym == mvm0::SYM_NONE);
  REQUIRE(rom.ins[7].args == std::vector<int>{0, 0});

  REQUIRE(rom.ins[8].name == "sys");
  REQUIRE(rom.ins[8].def_sym == mvm0::SYM_NONE);
  REQUIRE(rom.ins[8].use_sym == mvm0::SYM_NONE);
  REQUIRE(rom.ins[8].args == std::vector<int>{0});
}

TEST_CASE("exec call_add", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  CPU cpu(rom);
  cpu.init();

  REQUIRE(cpu.status() == CPU::Status::OK);
  REQUIRE(cpu.get_pc() == 1024);
  REQUIRE(cpu.get_reg(REG_SP) == 1024);

  REQUIRE(cpu.step() == 0);
  REQUIRE(cpu.get_pc() == 1027);

  REQUIRE(cpu.step() == 0);
  REQUIRE(cpu.get_pc() == 1028);
  REQUIRE(cpu.get_reg(0) == 12);

  REQUIRE(cpu.step() == 0);
  REQUIRE(cpu.get_pc() == 1029);
  REQUIRE(cpu.get_reg(1) == 45);

  REQUIRE(cpu.step() == 0);
  REQUIRE(cpu.get_pc() == 1025);
  REQUIRE(cpu.get_reg(REG_SP) == 1020);
  REQUIRE(read_u32(cpu, 1020) == 1030);

  REQUIRE(cpu.step() == 0);
  REQUIRE(cpu.get_pc() == 1026);
  REQUIRE(cpu.get_reg(0) == 57);
  REQUIRE(cpu.get_reg(1) == 45);

  REQUIRE(cpu.step() == 0);
  REQUIRE(cpu.get_pc() == 1030);
  REQUIRE(cpu.get_reg(REG_SP) == 1024);

  REQUIRE(cpu.step() == 0);
  REQUIRE(cpu.get_pc() == 1031);
  REQUIRE(cpu.get_reg(0) == 57);
  REQUIRE(cpu.get_reg(10) == 57);

  REQUIRE(cpu.step() == 0);
  REQUIRE(cpu.get_pc() == 1032);
  REQUIRE(cpu.get_reg(0) == 0);

  REQUIRE(cpu.step() == 1);
  REQUIRE(cpu.get_pc() == 1033);
  REQUIRE(cpu.status() == CPU::Status::NORMAL_EXIT);
}

TEST_CASE("exec call_sum", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_SUM);
  CPU cpu(rom);
  cpu.init();

  REQUIRE(cpu.status() == CPU::Status::OK);

  while (cpu.step() == 0)
    continue;

  REQUIRE(cpu.get_pc() == 1071);
  REQUIRE(cpu.get_reg(0) == 0);
  REQUIRE(cpu.get_reg(10) == 1608);
  REQUIRE(cpu.status() == CPU::Status::NORMAL_EXIT);
}

TEST_CASE("debug call_add infos", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();
  REQUIRE(db.get_state() == odb::Debugger::State::RUNNING_TOFINISH);

  for (std::size_t i = 0; i <= 14; ++i)
    REQUIRE(db_get_reg(db, i) == 0);
  REQUIRE(db_get_reg(db, 15) == 1024);
  REQUIRE(db_get_reg(db, 16) == 1024);
  REQUIRE(db_get_reg(db, 17) == 0);
  REQUIRE_THROWS(db_get_reg(db, 18));
  REQUIRE_THROWS(db_get_reg(db, 19));
  REQUIRE_THROWS(db_get_reg(db, -1));

  for (std::size_t i = 0; i <= 14; ++i) {
    auto infos = db.get_reg_infos(i);
    REQUIRE(infos.idx == i);
    REQUIRE(infos.name == "r" + std::to_string(i));
    REQUIRE(infos.size == 4);
    REQUIRE(infos.kind == odb::RegKind::general);
  }

  auto infos_sp = db.get_reg_infos(15);
  REQUIRE(infos_sp.idx == 15);
  REQUIRE(infos_sp.name == "sp");
  REQUIRE(infos_sp.size == 4);
  REQUIRE(infos_sp.kind == odb::RegKind::stack_pointer);

  auto infos_pc = db.get_reg_infos(16);
  REQUIRE(infos_pc.idx == 16);
  REQUIRE(infos_pc.name == "pc");
  REQUIRE(infos_pc.size == 4);
  REQUIRE(infos_pc.kind == odb::RegKind::program_counter);

  auto infos_zf = db.get_reg_infos(17);
  REQUIRE(infos_zf.idx == 17);
  REQUIRE(infos_zf.name == "zf");
  REQUIRE(infos_zf.size == 4);
  REQUIRE(infos_zf.kind == odb::RegKind::flags);

  REQUIRE_THROWS(db.get_reg_infos(18));
  REQUIRE_THROWS(db.get_reg_infos(19));
  REQUIRE_THROWS(db.get_reg_infos(-1));

  for (std::size_t i = 0; i <= 14; ++i)
    REQUIRE(db.find_reg_id("r" + std::to_string(i)) == i);
  REQUIRE(db.find_reg_id("sp") == 15);
  REQUIRE(db.find_reg_id("pc") == 16);
  REQUIRE(db.find_reg_id("zf") == 17);
  REQUIRE_THROWS(db.find_reg_id("r15"));
  REQUIRE_THROWS(db.find_reg_id("lol"));
  REQUIRE_THROWS(db.find_reg_id(""));
  REQUIRE(db.registers_count() == 18);

  REQUIRE(db.list_regs(odb::RegKind::general) ==
          std::vector<odb::vm_reg_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                                     13, 14});
  REQUIRE(db.list_regs(odb::RegKind::stack_pointer) ==
          std::vector<odb::vm_reg_t>{15});
  REQUIRE(db.list_regs(odb::RegKind::program_counter) ==
          std::vector<odb::vm_reg_t>{16});
  REQUIRE(db.list_regs(odb::RegKind::flags) == std::vector<odb::vm_reg_t>{17});
  REQUIRE(db.list_regs(odb::RegKind::base_pointer) ==
          std::vector<odb::vm_reg_t>{});

  REQUIRE(db.get_memory_size() == 2048);

  REQUIRE(db.pointer_size() == 4);
  REQUIRE(db.integer_size() == 4);
  REQUIRE(db.use_opcode() == false);

  REQUIRE(db.get_symbols(0, 56) == std::vector<odb::vm_sym_t>{});
  REQUIRE(db.get_symbols(1024, 1) == std::vector<odb::vm_sym_t>{0});
  REQUIRE(db.get_symbols(1024, 1) == std::vector<odb::vm_sym_t>{0});
  REQUIRE(db.get_symbols(1024, 4) == std::vector<odb::vm_sym_t>{0, 1, 2});
  REQUIRE(db.get_symbols(1900, 97) == std::vector<odb::vm_sym_t>{});
  REQUIRE(db.get_symbols(1900, 148) == std::vector<odb::vm_sym_t>{});

  auto sym0 = db.get_symbol_infos(0);
  REQUIRE(sym0.idx == 0);
  REQUIRE(sym0.name == "_begin");
  REQUIRE(sym0.addr == 1024);

  auto sym1 = db.get_symbol_infos(1);
  REQUIRE(sym1.idx == 1);
  REQUIRE(sym1.name == "my_add");
  REQUIRE(sym1.addr == 1025);

  auto sym2 = db.get_symbol_infos(2);
  REQUIRE(sym2.idx == 2);
  REQUIRE(sym2.name == "_start");
  REQUIRE(sym2.addr == 1027);

  REQUIRE_THROWS(db.get_symbol_infos(3));
  REQUIRE_THROWS(db.get_symbol_infos(4));
  REQUIRE_THROWS(db.get_symbol_infos(-1));

  REQUIRE(db.get_symbol_at(1024) == 0);
  REQUIRE(db.get_symbol_at(1025) == 1);
  REQUIRE(db.get_symbol_at(1026) == odb::VM_SYM_NULL);
  REQUIRE(db.get_symbol_at(1027) == 2);
  REQUIRE(db.get_symbol_at(1028) == odb::VM_SYM_NULL);
  REQUIRE(db.get_symbol_at(1023) == odb::VM_SYM_NULL);
  REQUIRE(db.get_symbol_at(0) == odb::VM_SYM_NULL);

  REQUIRE(db.find_sym_id("_begin") == 0);
  REQUIRE(db.find_sym_id("my_add") == 1);
  REQUIRE(db.find_sym_id("_start") == 2);
  REQUIRE_THROWS(db.find_sym_id("nonop"));
  REQUIRE(db.symbols_count() == 3);

  odb::vm_size_t un;
  REQUIRE(db.get_code_text(1023, un) == "");
  REQUIRE(db.get_code_text(1024, un) == "b {2}");
  REQUIRE(db.get_code_text(1025, un) == "add r0 r1 r0");
  REQUIRE(db.get_code_text(1026, un) == "ret");
  REQUIRE(db.get_code_text(1027, un) == "movi 12 r0");
  REQUIRE(db.get_code_text(1028, un) == "movi 45 r1");
  REQUIRE(db.get_code_text(1029, un) == "call {1}");
  REQUIRE(db.get_code_text(1030, un) == "mov r0 r10");
  REQUIRE(db.get_code_text(1031, un) == "movi 0 r0");
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db.get_code_text(1033, un) == "");
  REQUIRE(db.get_code_text(2047, un) == "");
  REQUIRE_THROWS(db.get_code_text(2048, un));
  REQUIRE_THROWS(db.get_code_text(2049, un));
  REQUIRE_THROWS(db.get_code_text(-1, un));

  REQUIRE(db.get_execution_point() == 1024);
}

TEST_CASE("debug call_add step", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();
  REQUIRE(db.get_state() == odb::Debugger::State::RUNNING_TOFINISH);

  auto cs = db.get_call_stack();
  REQUIRE(cs.size() == 1);
  REQUIRE(cs[0].caller_start_addr == 1024);
  REQUIRE(db.get_execution_point() == 1024);
  REQUIRE(db.get_code_text(1024, un) == "b {2}");

  db.resume(odb::ResumeType::Step);
  REQUIRE(db.get_state() == odb::Debugger::State::RUNNING_STEP);

  REQUIRE(cpu.step() == 0);
  db.on_update();
  REQUIRE(db.get_execution_point() == 1027);
  REQUIRE(db.get_code_text(1027, un) == "movi 12 r0");

  db_step(db, cpu);
  REQUIRE(db.get_execution_point() == 1028);
  REQUIRE(db.get_code_text(1028, un) == "movi 45 r1");
  REQUIRE(db_get_reg(db, 0) == 12);

  db_step(db, cpu);
  REQUIRE(db.get_execution_point() == 1029);
  REQUIRE(db.get_code_text(1029, un) == "call {1}");
  REQUIRE(db_get_reg(db, 0) == 12);
  REQUIRE(db_get_reg(db, 1) == 45);

  db_step(db, cpu);
  REQUIRE(db.get_execution_point() == 1025);
  REQUIRE(db.get_code_text(1025, un) == "add r0 r1 r0");
  cs = db.get_call_stack();
  REQUIRE(cs.size() == 2);
  REQUIRE(cs[1].caller_start_addr == 1025);
  REQUIRE(cs[0].call_addr == 1029);
  REQUIRE(cs[0].caller_start_addr == 1024);
  REQUIRE(db_get_reg(db, 15) == 1020);
  REQUIRE(db_read_u32(db, 1020) == 1030);

  db_step(db, cpu);
  REQUIRE(db.get_execution_point() == 1026);
  REQUIRE(db.get_code_text(1026, un) == "ret");
  REQUIRE(db_get_reg(db, 0) == 57);
  REQUIRE(db_get_reg(db, 1) == 45);

  db_step(db, cpu);
  REQUIRE(db.get_execution_point() == 1030);
  REQUIRE(db.get_code_text(1030, un) == "mov r0 r10");
  cs = db.get_call_stack();
  REQUIRE(cs.size() == 1);
  REQUIRE(cs[0].caller_start_addr == 1024);
  REQUIRE(db_get_reg(db, 15) == 1024);

  db_step(db, cpu);
  REQUIRE(db.get_execution_point() == 1031);
  REQUIRE(db.get_code_text(1031, un) == "movi 0 r0");
  REQUIRE(db_get_reg(db, 0) == 57);
  REQUIRE(db_get_reg(db, 10) == 57);

  db_step(db, cpu);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 10) == 57);

  db.resume(odb::ResumeType::Step);
  REQUIRE(cpu.step() == 1);
  db.on_update();
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 10) == 57);
  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
}

TEST_CASE("debug call_add run_direct", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();
  db_resume(db, cpu, odb::ResumeType::ToFinish);

  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 10) == 57);
}

TEST_CASE("debug call_add step_over", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();

  db_resume(db, cpu, odb::ResumeType::StepOver);
  REQUIRE(db.get_execution_point() == 1027);
  REQUIRE(db.get_code_text(1027, un) == "movi 12 r0");

  db_resume(db, cpu, odb::ResumeType::StepOver);
  REQUIRE(db.get_execution_point() == 1028);
  REQUIRE(db.get_code_text(1028, un) == "movi 45 r1");

  db_resume(db, cpu, odb::ResumeType::StepOver);
  REQUIRE(db.get_execution_point() == 1029);
  REQUIRE(db.get_code_text(1029, un) == "call {1}");
  REQUIRE(db_get_reg(db, 0) == 12);
  REQUIRE(db_get_reg(db, 1) == 45);

  db_resume(db, cpu, odb::ResumeType::StepOver);
  REQUIRE(db.get_execution_point() == 1030);
  REQUIRE(db.get_code_text(1030, un) == "mov r0 r10");
  REQUIRE(db_get_reg(db, 0) == 57);
  REQUIRE(db_get_reg(db, 1) == 45);

  db_resume(db, cpu, odb::ResumeType::StepOver);
  REQUIRE(db.get_execution_point() == 1031);
  REQUIRE(db.get_code_text(1031, un) == "movi 0 r0");

  db_resume(db, cpu, odb::ResumeType::StepOver);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");

  db_resume(db, cpu, odb::ResumeType::StepOver);
  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 10) == 57);
}

TEST_CASE("debug call_add step_out", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();

  db_resume(db, cpu, odb::ResumeType::Step);
  REQUIRE(db.get_execution_point() == 1027);
  REQUIRE(db.get_code_text(1027, un) == "movi 12 r0");

  db_resume(db, cpu, odb::ResumeType::Step);
  REQUIRE(db.get_execution_point() == 1028);

  db_resume(db, cpu, odb::ResumeType::Step);
  REQUIRE(db.get_execution_point() == 1029);
  REQUIRE(db.get_call_stack().size() == 1);

  db_resume(db, cpu, odb::ResumeType::Step);
  REQUIRE(db.get_execution_point() == 1025);
  REQUIRE(db.get_code_text(1025, un) == "add r0 r1 r0");
  REQUIRE(db.get_call_stack().size() == 2);

  db_resume(db, cpu, odb::ResumeType::StepOut);
  REQUIRE(db.get_execution_point() == 1030);
  REQUIRE(db.get_code_text(1030, un) == "mov r0 r10");
  REQUIRE(db_get_reg(db, 0) == 57);
  REQUIRE(db_get_reg(db, 1) == 45);
  REQUIRE(db.get_call_stack().size() == 1);

  db_resume(db, cpu, odb::ResumeType::StepOut);
  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 10) == 57);
}

TEST_CASE("debug continue no bp", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();
  db_resume(db, cpu, odb::ResumeType::Continue);

  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 10) == 57);
}

TEST_CASE("debug continue bkps", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();
  db.add_breakpoint(1025);
  db.add_breakpoint(1028);
  db.add_breakpoint(1031);
  db.add_breakpoint(1032);

  db.add_breakpoint(0);
  db.add_breakpoint(1023);
  db.add_breakpoint(1024);
  db.add_breakpoint(2047);
  REQUIRE_THROWS(db.add_breakpoint(1028));
  REQUIRE_THROWS(db.add_breakpoint(1032));
  REQUIRE_THROWS(db.add_breakpoint(2048));
  REQUIRE_THROWS(db.add_breakpoint(2049));
  REQUIRE_THROWS(db.add_breakpoint(-1));

  REQUIRE(db.has_breakpoint(1023));
  REQUIRE(db.has_breakpoint(1024));
  REQUIRE(db.has_breakpoint(1025));
  REQUIRE(!db.has_breakpoint(1026));
  db.del_breakpoint(1023);
  REQUIRE(!db.has_breakpoint(1023));
  REQUIRE_THROWS(db.del_breakpoint(1026));

  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_execution_point() == 1028);
  REQUIRE(db.get_code_text(1028, un) == "movi 45 r1");

  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_execution_point() == 1025);
  REQUIRE(db.get_code_text(1025, un) == "add r0 r1 r0");
  REQUIRE(db_get_reg(db, 0) == 12);
  REQUIRE(db_get_reg(db, 1) == 45);

  db.del_breakpoint(1031);
  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 10) == 57);
  REQUIRE(db.get_state() != odb::Debugger::State::EXIT);

  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
}

TEST_CASE("debug call_add step_out bkps", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();
  db.add_breakpoint(1025);
  db.add_breakpoint(1026);
  db.add_breakpoint(1032);

  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_execution_point() == 1025);
  REQUIRE(db.get_code_text(1025, un) == "add r0 r1 r0");
  REQUIRE(db_get_reg(db, 0) == 12);
  REQUIRE(db_get_reg(db, 1) == 45);

  db_resume(db, cpu, odb::ResumeType::StepOut);
  REQUIRE(db.get_execution_point() == 1026);
  REQUIRE(db.get_code_text(1026, un) == "ret");
  REQUIRE(db_get_reg(db, 0) == 57);
  REQUIRE(db_get_reg(db, 1) == 45);

  db_resume(db, cpu, odb::ResumeType::StepOut);
  REQUIRE(db.get_execution_point() == 1030);
  REQUIRE(db.get_code_text(1030, un) == "mov r0 r10");

  db_resume(db, cpu, odb::ResumeType::StepOut);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 10) == 57);
  REQUIRE(db.get_state() != odb::Debugger::State::EXIT);

  db_resume(db, cpu, odb::ResumeType::StepOut);
  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
}

TEST_CASE("debug call_add step_over bkps", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();
  db.add_breakpoint(1029);
  db.add_breakpoint(1026);
  db.add_breakpoint(1032);

  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_execution_point() == 1029);
  REQUIRE(db.get_code_text(1029, un) == "call {1}");
  REQUIRE(db_get_reg(db, 0) == 12);
  REQUIRE(db_get_reg(db, 1) == 45);

  db_resume(db, cpu, odb::ResumeType::StepOver);
  REQUIRE(db.get_execution_point() == 1026);
  REQUIRE(db.get_code_text(1026, un) == "ret");
  REQUIRE(db_get_reg(db, 0) == 57);
  REQUIRE(db_get_reg(db, 1) == 45);

  db_resume(db, cpu, odb::ResumeType::StepOver);
  REQUIRE(db.get_execution_point() == 1030);
  REQUIRE(db.get_code_text(1030, un) == "mov r0 r10");

  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 10) == 57);
  REQUIRE(db.get_state() != odb::Debugger::State::EXIT);

  db_resume(db, cpu, odb::ResumeType::StepOver);
  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
}

TEST_CASE("debug call_add step bkps", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();
  for (odb::vm_ptr_t it = 1024; it < 1056; ++it)
    db.add_breakpoint(it);

  db_resume(db, cpu, odb::ResumeType::Step);
  REQUIRE(db.get_execution_point() == 1027);
  REQUIRE(db.get_code_text(1027, un) == "movi 12 r0");

  db_resume(db, cpu, odb::ResumeType::ToFinish);
  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 1) == 45);
  REQUIRE(db_get_reg(db, 10) == 57);
}

TEST_CASE("debug call_add run_direct bkps", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();
  for (odb::vm_ptr_t it = 1024; it < 1056; ++it)
    db.add_breakpoint(it);

  db_resume(db, cpu, odb::ResumeType::ToFinish);
  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 1) == 45);
  REQUIRE(db_get_reg(db, 10) == 57);
}

TEST_CASE("debug call_add run_direct change reg", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();

  db.add_breakpoint(1025);
  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_execution_point() == 1025);
  REQUIRE(db.get_code_text(1025, un) == "add r0 r1 r0");
  REQUIRE(db_get_reg(db, 0) == 12);
  REQUIRE(db_get_reg(db, 1) == 45);
  db_set_reg(db, 0, 19);
  REQUIRE(db_get_reg(db, 0) == 19);
  REQUIRE(db_get_reg(db, 1) == 45);

  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 1) == 45);
  REQUIRE(db_get_reg(db, 10) == 64);
}

TEST_CASE("debug call_add run_direct change ret addr", "") {
  using namespace mvm0;
  auto rom = parse_file(PATH_CALL_ADD);
  odb::vm_size_t un;
  CPU cpu(rom);
  cpu.init();
  odb::Debugger db(std::make_unique<VMApi>(cpu));
  db.on_init();

  db.add_breakpoint(1025);
  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_execution_point() == 1025);
  REQUIRE(db.get_code_text(1025, un) == "add r0 r1 r0");
  REQUIRE(db_get_reg(db, 0) == 12);
  REQUIRE(db_get_reg(db, 1) == 45);
  db.del_breakpoint(1025);
  REQUIRE(db_get_reg(db, 15) == 1020);
  REQUIRE(db_read_u32(db, 1020) == 1030);
  db_write_u32(db, 1020, 1029); // set ret addr to call @my_add
  REQUIRE(db_read_u32(db, 1020) == 1029);

  db_resume(db, cpu, odb::ResumeType::Continue);
  REQUIRE(db.get_state() == odb::Debugger::State::EXIT);
  REQUIRE(db.get_execution_point() == 1032);
  REQUIRE(db.get_code_text(1032, un) == "sys 0");
  REQUIRE(db_get_reg(db, 0) == 0);
  REQUIRE(db_get_reg(db, 1) == 45);
  REQUIRE(db_get_reg(db, 10) == 102);
}
