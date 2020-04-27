#include "utils.hh"

namespace {

void test_call_add_preg(SimpleCLIMode mode) {
  const char *cmds = ""
                     "preg u32 %0 %1\n"
                     "preg u32 %r0 %r1\n"
                     "preg u32 %15 %16\n"
                     "preg u32 %sp %pc\n"
                     "step\n"
                     "step\n"
                     "preg u32 %0 %1\n"
                     "preg u32 %r0 %r1\n"
                     "preg u32 %15 %16 %sp %pc\n"
                     "step\n"
                     "preg u32 %0 %1 %r0 %r1\n"
                     "preg u32 %18\n"
                     "preg u32 %r15\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 26);
  REQUIRE(vals[0] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[1] == "%0: 0");
  REQUIRE(vals[2] == "%1: 0");
  REQUIRE(vals[3] == "%r0: 0");
  REQUIRE(vals[4] == "%r1: 0");
  REQUIRE(vals[5] == "%15: 1024");
  REQUIRE(vals[6] == "%16: 1024");
  REQUIRE(vals[7] == "%sp: 1024");
  REQUIRE(vals[8] == "%pc: 1024");
  REQUIRE(vals[9] == "program stopped at 0x403 (<_begin> + 0x3)");
  REQUIRE(vals[10] == "program stopped at 0x404 (<_begin> + 0x4)");
  REQUIRE(vals[11] == "%0: 12");
  REQUIRE(vals[12] == "%1: 0");
  REQUIRE(vals[13] == "%r0: 12");
  REQUIRE(vals[14] == "%r1: 0");
  REQUIRE(vals[15] == "%15: 1024");
  REQUIRE(vals[16] == "%16: 1028");
  REQUIRE(vals[17] == "%sp: 1024");
  REQUIRE(vals[18] == "%pc: 1028");
  REQUIRE(vals[19] == "program stopped at 0x405 (<_begin> + 0x5)");
  REQUIRE(vals[20] == "%0: 12");
  REQUIRE(vals[21] == "%1: 45");
  REQUIRE(vals[22] == "%r0: 12");
  REQUIRE(vals[23] == "%r1: 45");
  REQUIRE(vals[24] == "Error: invalid register index");
  REQUIRE(vals[25] == "Error: Invalid register name");
}

void test_call_add_sreg(SimpleCLIMode mode) {
  const char *cmds = ""
                     "b @my_add\n"
                     "continue\n"
                     "preg u32 %r0 %r1\n"
                     "sreg u32 %0 49 %r1 27\n"
                     "preg u32 %r0 %r1\n"
                     "step\n"
                     "preg u32 %r0 %r1\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 10);
  REQUIRE(vals[3] == "%r0: 12");
  REQUIRE(vals[4] == "%r1: 45");
  REQUIRE(vals[5] == "%r0: 49");
  REQUIRE(vals[6] == "%r1: 27");
  REQUIRE(vals[8] == "%r0: 76");
  REQUIRE(vals[9] == "%r1: 27");
}

void test_call_add_pregi(SimpleCLIMode mode) {
  const char *cmds = ""
                     "pregi %0 %r0\n"
                     "pregi %r1\n"
                     "pregi %r2\n"
                     "pregi %r3\n"
                     "pregi %r4\n"
                     "pregi %r5\n"
                     "pregi %r6\n"
                     "pregi %r7\n"
                     "pregi %r8\n"
                     "pregi %r9\n"
                     "pregi %r10\n"
                     "pregi %r11\n"
                     "pregi %r12\n"
                     "pregi %r13\n"
                     "pregi %r14\n"
                     "pregi %sp\n"
                     "pregi %pc\n"
                     "pregi %zf\n"
                     "pregi %18\n"
                     "pregi %r15\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 60);
  REQUIRE(vals[1] == "Register r0 (#0):");
  REQUIRE(vals[2] == "  General purpose register");
  REQUIRE(vals[3] == "  size: 4 bytes");
  REQUIRE(vals[4] == "Register r0 (#0):");
  REQUIRE(vals[5] == "  General purpose register");
  REQUIRE(vals[6] == "  size: 4 bytes");
  REQUIRE(vals[7] == "Register r1 (#1):");
  REQUIRE(vals[8] == "  General purpose register");
  REQUIRE(vals[9] == "  size: 4 bytes");
  REQUIRE(vals[49] == "Register sp (#15):");
  REQUIRE(vals[50] == "  Stack pointer");
  REQUIRE(vals[51] == "  size: 4 bytes");
  REQUIRE(vals[52] == "Register pc (#16):");
  REQUIRE(vals[53] == "  Program counter");
  REQUIRE(vals[54] == "  size: 4 bytes");
  REQUIRE(vals[55] == "Register zf (#17):");
  REQUIRE(vals[56] == "  Flags register");
  REQUIRE(vals[57] == "  size: 4 bytes");
  REQUIRE(vals[58] == "Error: invalid register index");
  REQUIRE(vals[59] == "Error: Invalid register name");
}

void test_call_add_pmem(SimpleCLIMode mode) {
  const char *cmds = ""
                     "pmem u32 1012 5\n"
                     "b @my_add\n"
                     "continue\n"
                     "pmem u32 1012 5\n"
                     "pmem u16 1018 5\n"
                     "pmem u8 1018 5\n"
                     "pmem u8 12046 2\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 8);
  REQUIRE(vals[1] == "0 0 0 0 0 ");
  REQUIRE(vals[4] == "0 0 1030 0 0 ");
  REQUIRE(vals[5] == "0 1030 0 0 0 ");
  REQUIRE(vals[6] == "0 0 6 4 0 ");
  REQUIRE(vals[7] == "Error: Memory address out of range (0-2048)");
}

void test_call_add_psym(SimpleCLIMode mode) {
  const char *cmds = ""
                     "psym @_begin\n"
                     "psym @my_add\n"
                     "psym @_start\n"
                     "psym @1\n"
                     "psym @2\n"
                     "psym @0\n"
                     "psym @foo\n"
                     "psym @3\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 15);
  REQUIRE(vals[1] == "Symbol _begin (#0):");
  REQUIRE(vals[2] == "  address: 0x400");
  REQUIRE(vals[3] == "Symbol my_add (#1):");
  REQUIRE(vals[4] == "  address: 0x401");
  REQUIRE(vals[5] == "Symbol _start (#2):");
  REQUIRE(vals[6] == "  address: 0x403");
  REQUIRE(vals[7] == "Symbol my_add (#1):");
  REQUIRE(vals[8] == "  address: 0x401");
  REQUIRE(vals[9] == "Symbol _start (#2):");
  REQUIRE(vals[10] == "  address: 0x403");
  REQUIRE(vals[11] == "Symbol _begin (#0):");
  REQUIRE(vals[12] == "  address: 0x400");
  REQUIRE(vals[13] == "Error: Invalid symbol name");
  REQUIRE(vals[14] == "Error: Invalid symbol index");
}

void test_call_add_b(SimpleCLIMode mode) {
  const char *cmds = ""
                     "b 1031\n"
                     "b @my_add\n"
                     "continue\n"
                     "continue\n"
                     "b @foo\n"
                     "b @0\n"
                     "b @1\n"
                     "b @4\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 9);
  REQUIRE(vals[0] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[1] == "Inserted breakpoint at `0x407'");
  REQUIRE(vals[2] == "Inserted breakpoint at `0x401'");
  REQUIRE(vals[3] == "program stopped at 0x401 (<my_add> + 0x0)");
  REQUIRE(vals[4] == "program stopped at 0x407 (<_begin> + 0x7)");
  REQUIRE(vals[5] == "Error: Invalid symbol name");
  REQUIRE(vals[6] == "Inserted breakpoint at `0x400'");
  REQUIRE(vals[7] ==
          "Error: cannot add breakpoint: There is already one at this address");
  REQUIRE(vals[8] == "Error: Invalid symbol index");
}

void test_call_add_delb(SimpleCLIMode mode) {
  const char *cmds = ""
                     "b 1031\n"
                     "b @my_add\n"
                     "continue\n"
                     "delb @1\n"
                     "delb 1031\n"
                     "continue\n"
                     "delb @foo\n"
                     "delb @4\n"
                     "delb @2\n"
                     "delb @my_add\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 11);
  REQUIRE(vals[0] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[1] == "Inserted breakpoint at `0x407'");
  REQUIRE(vals[2] == "Inserted breakpoint at `0x401'");
  REQUIRE(vals[3] == "program stopped at 0x401 (<my_add> + 0x0)");
  REQUIRE(vals[4] == "Removed breakpoint at `0x401'");
  REQUIRE(vals[5] == "Removed breakpoint at `0x407'");
  REQUIRE(vals[6] == "Program exited normally at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[7] == "Error: Invalid symbol name");
  REQUIRE(vals[8] == "Error: Invalid symbol index");
  REQUIRE(vals[9] ==
          "Error: cannot delete breakpoint: there is none at this address");
  REQUIRE(vals[10] ==
          "Error: cannot delete breakpoint: there is none at this address");
}

void test_call_add_continue(SimpleCLIMode mode) {
  const char *cmds = ""
                     "b 1027\n"
                     "b 1028\n"
                     "b @my_add\n"
                     "b 1031\n"
                     "continue\n"
                     "continue\n"
                     "continue\n"
                     "continue\n"
                     "continue\n"
                     "continue\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 11);
  REQUIRE(vals[0] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[5] == "program stopped at 0x403 (<_begin> + 0x3)");
  REQUIRE(vals[6] == "program stopped at 0x404 (<_begin> + 0x4)");
  REQUIRE(vals[7] == "program stopped at 0x401 (<my_add> + 0x0)");
  REQUIRE(vals[8] == "program stopped at 0x407 (<_begin> + 0x7)");
  REQUIRE(vals[9] == "Program exited normally at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[10] ==
          "Error: cannot resume execution: program already finished");
}

void test_call_add_continue_short(SimpleCLIMode mode) {
  const char *cmds = ""
                     "b 1027\n"
                     "b 1028\n"
                     "b 1032\n"
                     "c\n"
                     "c\n"
                     "c\n"
                     "c\n"
                     "c\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 9);
  REQUIRE(vals[0] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[4] == "program stopped at 0x403 (<_begin> + 0x3)");
  REQUIRE(vals[5] == "program stopped at 0x404 (<_begin> + 0x4)");
  REQUIRE(vals[6] == "program stopped at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[7] == "Program exited normally at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[8] ==
          "Error: cannot resume execution: program already finished");
}

void test_call_add_step(SimpleCLIMode mode) {
  const char *cmds = ""
                     "step\n"
                     "step\n"
                     "step\n"
                     "step\n"
                     "step\n"
                     "step\n"
                     "step\n"
                     "step\n"
                     "step\n"
                     "step\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 11);
  REQUIRE(vals[0] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[1] == "program stopped at 0x403 (<_begin> + 0x3)");
  REQUIRE(vals[2] == "program stopped at 0x404 (<_begin> + 0x4)");
  REQUIRE(vals[3] == "program stopped at 0x405 (<_begin> + 0x5)");
  REQUIRE(vals[4] == "program stopped at 0x401 (<my_add> + 0x0)");
  REQUIRE(vals[5] == "program stopped at 0x402 (<my_add> + 0x1)");
  REQUIRE(vals[6] == "program stopped at 0x406 (<_begin> + 0x6)");
  REQUIRE(vals[7] == "program stopped at 0x407 (<_begin> + 0x7)");
  REQUIRE(vals[8] == "program stopped at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[9] == "Program exited normally at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[10] ==
          "Error: cannot resume execution: program already finished");
}

void test_call_add_step_short(SimpleCLIMode mode) {
  const char *cmds = ""
                     "s\n"
                     "s\n"
                     "s\n"
                     "s\n"
                     "s\n"
                     "s\n"
                     "s\n"
                     "s\n"
                     "s\n"
                     "s\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 11);
  REQUIRE(vals[0] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[1] == "program stopped at 0x403 (<_begin> + 0x3)");
  REQUIRE(vals[2] == "program stopped at 0x404 (<_begin> + 0x4)");
  REQUIRE(vals[3] == "program stopped at 0x405 (<_begin> + 0x5)");
  REQUIRE(vals[4] == "program stopped at 0x401 (<my_add> + 0x0)");
  REQUIRE(vals[5] == "program stopped at 0x402 (<my_add> + 0x1)");
  REQUIRE(vals[6] == "program stopped at 0x406 (<_begin> + 0x6)");
  REQUIRE(vals[7] == "program stopped at 0x407 (<_begin> + 0x7)");
  REQUIRE(vals[8] == "program stopped at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[9] == "Program exited normally at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[10] ==
          "Error: cannot resume execution: program already finished");
}

void test_call_add_next(SimpleCLIMode mode) {
  const char *cmds = ""
                     "next\n"
                     "next\n"
                     "next\n"
                     "next\n"
                     "next\n"
                     "next\n"
                     "next\n"
                     "next\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 9);
  REQUIRE(vals[0] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[1] == "program stopped at 0x403 (<_begin> + 0x3)");
  REQUIRE(vals[2] == "program stopped at 0x404 (<_begin> + 0x4)");
  REQUIRE(vals[3] == "program stopped at 0x405 (<_begin> + 0x5)");
  REQUIRE(vals[4] == "program stopped at 0x406 (<_begin> + 0x6)");
  REQUIRE(vals[5] == "program stopped at 0x407 (<_begin> + 0x7)");
  REQUIRE(vals[6] == "program stopped at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[7] == "Program exited normally at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[8] ==
          "Error: cannot resume execution: program already finished");
}

void test_call_add_next_short(SimpleCLIMode mode) {
  const char *cmds = ""
                     "n\n"
                     "n\n"
                     "n\n"
                     "n\n"
                     "n\n"
                     "n\n"
                     "n\n"
                     "n\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 9);
  REQUIRE(vals[0] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[1] == "program stopped at 0x403 (<_begin> + 0x3)");
  REQUIRE(vals[2] == "program stopped at 0x404 (<_begin> + 0x4)");
  REQUIRE(vals[3] == "program stopped at 0x405 (<_begin> + 0x5)");
  REQUIRE(vals[4] == "program stopped at 0x406 (<_begin> + 0x6)");
  REQUIRE(vals[5] == "program stopped at 0x407 (<_begin> + 0x7)");
  REQUIRE(vals[6] == "program stopped at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[7] == "Program exited normally at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[8] ==
          "Error: cannot resume execution: program already finished");
}

void test_call_add_finish(SimpleCLIMode mode) {
  const char *cmds = ""
                     "b @my_add\n"
                     "finish\n"
                     "finish\n"
                     "finish\n"
                     "finish\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 6);
  REQUIRE(vals[0] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[2] == "program stopped at 0x401 (<my_add> + 0x0)");
  REQUIRE(vals[3] == "program stopped at 0x406 (<_begin> + 0x6)");
  REQUIRE(vals[4] == "Program exited normally at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[5] ==
          "Error: cannot resume execution: program already finished");
}

void test_call_add_finish_short(SimpleCLIMode mode) {
  const char *cmds = ""
                     "b @my_add\n"
                     "fin\n"
                     "fin\n"
                     "fin\n"
                     "fin\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 6);
  REQUIRE(vals[0] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[2] == "program stopped at 0x401 (<my_add> + 0x0)");
  REQUIRE(vals[3] == "program stopped at 0x406 (<_begin> + 0x6)");
  REQUIRE(vals[4] == "Program exited normally at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[5] ==
          "Error: cannot resume execution: program already finished");
}

void test_call_add_state(SimpleCLIMode mode) {
  const char *cmds = ""
                     "state\n"
                     "step\n"
                     "state\n"
                     "continue\n"
                     "state\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 6);
  REQUIRE(vals[0] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[1] == "program stopped at 0x400 (<_begin> + 0x0)");
  REQUIRE(vals[2] == "program stopped at 0x403 (<_begin> + 0x3)");
  REQUIRE(vals[3] == "program stopped at 0x403 (<_begin> + 0x3)");
  REQUIRE(vals[4] == "Program exited normally at 0x408 (<_begin> + 0x8)");
  REQUIRE(vals[5] == "Program exited normally at 0x408 (<_begin> + 0x8)");
}

void test_call_add_bt(SimpleCLIMode mode) {
  const char *cmds = ""
                     "b @my_add\n"
                     "bt\n"
                     "continue\n"
                     "bt\n"
                     "finish\n"
                     "bt\n"
                     "continue\n"
                     "bt\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_ADD, cmds), '\n');
  REQUIRE(vals.size() == 10);
  REQUIRE(vals[2] == "0x400 (<_begin> + 0x0)");
  REQUIRE(vals[4] == "0x401 (<my_add> + 0x0)");
  REQUIRE(vals[5] == "0x405 (<_begin> + 0x5)");
  REQUIRE(vals[7] == "0x406 (<_begin> + 0x6)");
  REQUIRE(vals[9] == "0x408 (<_begin> + 0x8)");
}

} // namespace

TEST_CASE("simplecli_on_server call_add preg", "") {
  test_call_add_preg(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add sreg", "") {
  test_call_add_sreg(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add pregi", "") {
  test_call_add_pregi(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add pmem", "") {
  test_call_add_pmem(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add psym", "") {
  test_call_add_psym(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add b", "") {
  test_call_add_b(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add delb", "") {
  test_call_add_delb(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add continue", "") {
  test_call_add_continue(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add continue_short", "") {
  test_call_add_continue_short(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add step", "") {
  test_call_add_step(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add step_short", "") {
  test_call_add_step_short(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add next", "") {
  test_call_add_next(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add next_short", "") {
  test_call_add_next_short(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add finish", "") {
  test_call_add_finish(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add finish_short", "") {
  test_call_add_finish_short(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add state", "") {
  test_call_add_state(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_add bt", "") {
  test_call_add_bt(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_with_tcp call_add preg", "") {
  test_call_add_preg(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add sreg", "") {
  test_call_add_sreg(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add pregi", "") {
  test_call_add_pregi(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add pmem", "") {
  test_call_add_pmem(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add psym", "") {
  test_call_add_psym(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add b", "") {
  test_call_add_b(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add delb", "") {
  test_call_add_delb(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add continue", "") {
  test_call_add_continue(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add continue_short", "") {
  test_call_add_continue_short(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add step", "") {
  test_call_add_step(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add step_short", "") {
  test_call_add_step_short(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add next", "") {
  test_call_add_next(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add next_short", "") {
  test_call_add_next_short(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add finish", "") {
  test_call_add_finish(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add finish_short", "") {
  test_call_add_finish_short(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add state", "") {
  test_call_add_state(SimpleCLIMode::WITH_TCP);
}

TEST_CASE("simplecli_with_tcp call_add bt", "") {
  test_call_add_bt(SimpleCLIMode::WITH_TCP);
}
