#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>

#include "odb/client/db-client-impl-data.hh"
#include "odb/client/tcp-data-client.hh"
#include "odb/mess/db-client.hh"
#include "odb/mess/simple-cli-client.hh"

void wait_stop(odb::DBClient &db_client) {

  while (1) {
    db_client.check_stopped();
    auto state = db_client.state();
    if (state != odb::DBClient::State::VM_RUNNING)
      break;

    auto sl_dur = std::chrono::milliseconds(10);
    std::this_thread::sleep_for(sl_dur);
  }
}

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

  bool print_state = true;

  while (1) {
    // Check if still connected
    auto state = db_client.state();
    if (state != odb::DBClient::State::VM_STOPPED &&
        state != odb::DBClient::State::VM_RUNNING)
      break;

    // If VM running, wait for it to stop (need to sends requests at interval
    // to know if still running)
    if (state == odb::DBClient::State::VM_RUNNING) {
      wait_stop(db_client);
      print_state = true;
    }

    if (print_state)
      std::cout << cli.exec("state");

    if (is_tty) {
      std::cout << "> ";
      std::cout.flush();
    }

    std::string cmd;
    std::getline(std::cin, cmd);
    if (cmd.empty())
      break;

#ifdef ODB_COMM_LOGS
    auto t1 = std::chrono::high_resolution_clock::now();
#endif
    std::string out = cli.exec(cmd);
#ifdef ODB_COMM_LOGS
    auto t2 = std::chrono::high_resolution_clock::now();
#endif

    std::cout << out;
    if (!out.empty() && out.back() != '\n')
      std::cout << std::endl;
    print_state = false;

#ifdef ODB_COMM_LOGS
    auto cmd_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << "Command executed in " << cmd_ms << "ms.\n";
#endif
  }

  return 0;
}
