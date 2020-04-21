#include "utils.hh"

namespace {

void test_call_sum_pmem(SimpleCLIMode mode) {
  const char *cmds = ""
                     "b @arr_sum\n"
                     "pmem u32 600 7\n"
                     "continue\n"
                     "pmem u32 600 7\n"
                     "continue\n"
                     "preg u32 %r10\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_SUM, cmds), '\n');
  REQUIRE(vals.size() == 7);
  REQUIRE(vals[2] == "0 0 0 0 0 0 0 ");
  REQUIRE(vals[4] == "17 87 6 489 197 812 0 ");
  REQUIRE(vals[6] == "%r10: 1608");
}

void test_call_sum_smem(SimpleCLIMode mode) {
  const char *cmds = ""
                     "b @arr_sum\n"
                     "continue\n"
                     "pmem u32 600 7\n"
                     "smem u32 612 13056 1000\n"
                     "pmem u32 600 7\n"
                     "continue\n"
                     "preg u32 %r10\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_SUM, cmds), '\n');
  REQUIRE(vals.size() == 7);
  REQUIRE(vals[3] == "17 87 6 489 197 812 0 ");
  REQUIRE(vals[4] == "17 87 6 13056 1000 812 0 ");
  REQUIRE(vals[6] == "%r10: 14978");
}

void test_call_sum_pmem_bases(SimpleCLIMode mode) {
  const char *cmds = ""
                     "c\n"
                     "pmem u32 600 7\n"
                     "pmem u32 0x25c 2\n"
                     "pmem u32 01140 2\n"
                     "pmem u32 0b1001100100 2\n";
  auto vals = str_split(run_simplecli(mode, PATH_CALL_SUM, cmds), '\n');
  REQUIRE(vals.size() == 6);
  REQUIRE(vals[2] == "17 87 6 489 197 812 0 ");
  REQUIRE(vals[3] == "87 6 ");
  REQUIRE(vals[4] == "6 489 ");
  REQUIRE(vals[5] == "489 197 ");
}

} // namespace

TEST_CASE("simplecli_on_server call_sum pmem", "") {
  test_call_sum_pmem(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_sum smem", "") {
  test_call_sum_smem(SimpleCLIMode::ON_SERVER);
}

TEST_CASE("simplecli_on_server call_sum pmem_bases", "") {
  test_call_sum_pmem_bases(SimpleCLIMode::ON_SERVER);
}
