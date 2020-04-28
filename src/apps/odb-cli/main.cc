#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "cli.hh"

namespace {
// get next command to run
// returns empty if received EOF
std::string get_input() {
  while (true) {
    std::cout << "> ";
    std::cout.flush();

    // check if stdin EOF
    std::cin.peek();
    if (!std::cin.good())
      return {};

    std::string cmd;
    std::getline(std::cin, cmd);
    if (!cmd.empty())
      return cmd;
  }
}

} // namespace

int main(int argc, char **argv) {
  if (argc > 3) {
    std::cerr << "Usage: odb-cli [<hostname>] [<port>]\n";
    return 1;
  }

  CLI cli(argc, argv);

  while (1) {
    // Check if still connected
    if (!cli.next())
      break;

    if (cli.state_switched())
      std::cout << cli.exec("state");

    auto cmd = get_input();
    if (cmd.empty())
      break;
    std::string out = cli.exec(cmd);

    std::cout << out;
    if (!out.empty() && out.back() != '\n')
      std::cout << std::endl;
  }

  return 0;
}
