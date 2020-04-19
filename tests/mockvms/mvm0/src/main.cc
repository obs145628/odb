#include <iostream>

#include <odb/server/server-app.hh>

#include "../include/mvm0/cpu.hh"
#include "../include/mvm0/parser.hh"
#include "../include/mvm0/rom.hh"
#include "../include/mvm0/vm-api.hh"

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage: ./mock-mvm0-app <rom-file>" << std::endl;
    return 1;
  }

  auto rom = mvm0::parse_file(argv[1]);
  mvm0::CPU cpu(rom);
  cpu.init();
  odb::ServerApp db([&cpu]() { return std::make_unique<mvm0::VMApi>(cpu); });

  for (;;) {
    db.loop();
    int st = cpu.step();
    if (st != 0)
      break;
  }

  db.loop();

  return 0;
}
