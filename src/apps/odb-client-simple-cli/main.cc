#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>

#include "odb/client/db-client-impl-data.hh"
#include "odb/client/tcp-data-client.hh"
#include "odb/mess/db-client.hh"
#include "odb/mess/simple-cli-client.hh"

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Usage: odb-client-simple-cli <hostname> <port>\n";
    return 1;
  }

  auto hostname = argv[1];
  auto port = std::atoi(argv[2]);
  auto tcp_cli = std::make_unique<odb::TCPDataClient>(hostname, port);
  auto tcp_impl = std::make_unique<odb::DBClientImplData>(std::move(tcp_cli));
  odb::DBClient db_client(std::move(tcp_impl));
  odb::SimpleCLIClient cli(db_client);
  bool is_tty = isatty(fileno(stdin));

  db_client.connect();

  while (1) {

    if (is_tty) {
      std::cout << "> ";
      std::cout.flush();
    }

    std::string cmd;
    std::getline(std::cin, cmd);
    if (cmd.empty())
      break;
    std::string out = cli.exec(cmd);

    std::cout << out;
    if (!out.empty() && out.back() != '\n')
      std::cout << std::endl;
  }

  return 0;
}
