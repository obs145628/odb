#include <catch2/catch.hpp>

#include "../include/mvm0/cpu.hh"
#include "../include/mvm0/parser.hh"
#include "../include/mvm0/rom.hh"
#include "../include/mvm0/vm-api.hh"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#include <odb/server/debugger.hh>

#define BIN_MVM std::string(BUILD_DIR "bin/mock-mvm0-app")
#define BIN_ODB_CLIENT std::string(BUILD_DIR "bin/odb-client-simple-cli")
#define PATH_CALL_ADD (MVM0_EXS_DIR + std::string("call_add.vv"))
#define PATH_CALL_SUM (MVM0_EXS_DIR + std::string("call_sum.vv"))

// use system (3) with async result
class SystemAsync {

public:
  SystemAsync(const std::string &cmd) : _cmd(cmd) {}

  ~SystemAsync() {
    if (_runner.joinable())
      _runner.join();
  }

  void start() { _runner = std::thread{&SystemAsync::run_code, this}; }

  void run_code() { _ret = std::system(_cmd.c_str()); }

  int wait() {
    _runner.join();
    return _ret;
  }

private:
  std::string _cmd;
  std::thread _runner;
  int _ret;
};

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

inline void write_file_str(const std::string &path, const std::string &data) {
  FILE *f = std::fopen(path.c_str(), "wb");
  REQUIRE(std::fwrite(data.c_str(), 1, data.size(), f) == data.size());
  std::fclose(f);
}

inline std::string read_file_str(const std::string &path) {
  FILE *f = std::fopen(path.c_str(), "rb");
  REQUIRE(std::fseek(f, 0, SEEK_END) == 0);
  size_t size = std::ftell(f);
  REQUIRE(std::fseek(f, 0, SEEK_SET) == 0);

  std::string res;
  res.resize(size);
  REQUIRE(std::fread(&res[0], 1, size, f) == size);
  std::fclose(f);
  return res;
}

enum class SimpleCLIMode { ON_SERVER, WITH_TCP };

inline std::string run_simplecli_onserver(const std::string &rom_path,
                                          const std::string &db_cmds) {
  const char *tmp_in_file = "/tmp/test_odb_simplecli_onserver_input.txt";
  const char *tmp_out_file = "/tmp/test_odb_simplecli_onserver_output.txt";

  std::remove(tmp_in_file);
  std::remove(tmp_out_file);

  write_file_str(tmp_in_file, db_cmds);

  std::string cmd = "ODB_CONF_ENABLED=1 ODB_CONF_MODE_SERVER_CLI=1 " + BIN_MVM +
                    " " + rom_path + " < " + std::string(tmp_in_file) + " > " +
                    std::string(tmp_out_file);
  REQUIRE(std::system(cmd.c_str()) == 0);
  return read_file_str(tmp_out_file);
}

inline std::string run_simplecli_withtcp(const std::string &rom_path,
                                         const std::string &db_cmds) {
  const char *tmp_in_file = "/tmp/test_odb_simplecli_withtcp_input.txt";
  const char *tmp_out_file = "/tmp/test_odb_simplecli_withtcp_output.txt";

  std::remove(tmp_in_file);
  std::remove(tmp_out_file);

  write_file_str(tmp_in_file, db_cmds);

  std::string cli_cmd = BIN_ODB_CLIENT + std::string(" 0.0.0.0 12644") + " < " +
                        std::string(tmp_in_file) + " > " +
                        std::string(tmp_out_file);
  std::string serv_cmd =
      "ODB_CONF_ENABLED=1 ODB_CONF_NOSTART=1 ODB_CONF_MODE_TCP=1 " + BIN_MVM +
      " " + rom_path;

  SystemAsync serv(serv_cmd);
  serv.start();
  // server must start before client
  std::this_thread::sleep_for(std::chrono::milliseconds(5));

  REQUIRE(std::system(cli_cmd.c_str()) == 0);
  REQUIRE(serv.wait() == 0);
  return read_file_str(tmp_out_file);
}

inline std::string run_simplecli(SimpleCLIMode mode,
                                 const std::string &rom_path,
                                 const std::string &db_cmds) {
  switch (mode) {
  case SimpleCLIMode::ON_SERVER:
    return run_simplecli_onserver(rom_path, db_cmds);
  case SimpleCLIMode::WITH_TCP:
    return run_simplecli_withtcp(rom_path, db_cmds);
  };

  return "???";
}

inline std::vector<std::string> str_split(const std::string &str, char sep) {
  std::istringstream is(str);
  std::vector<std::string> res;

  std::string line;
  while (std::getline(is, line, sep))
    res.push_back(line);
  return res;
}
