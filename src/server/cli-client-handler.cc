#include "odb/server/cli-client-handler.hh"

#include <cassert>
#include <cstdio>
#include <iostream>
#include <unistd.h>

#include "odb/server/db-client-impl-vmside.hh"

namespace odb {

CLIClientHandler::CLIClientHandler(Debugger &db)
    : ClientHandler(db), _db_client(std::make_unique<DBClientImplVMSide>(db)),
      _client(_db_client), _is_tty(isatty(fileno(stdin))) {}

void CLIClientHandler::setup_connection() {
  _db_client.connect();
  _client_connected();
}

void CLIClientHandler::run_command() {
  bool stopped = false;

  if (_db_client.state() == DBClient::State::VM_RUNNING) {
    _db_client.check_stopped();
    stopped = true;
  }
  assert(_db_client.state() == DBClient::State::VM_STOPPED);

  if (stopped)
    std::cout << _client.exec("state");

  if (_is_tty) {
    std::cout << "> ";
    std::cout.flush();
  }

  // Disconnect is stdin closed
  std::cin.peek();
  if (!std::cin.good())
    _client_disconnected();

  // Read and exec one command
  std::string cmd;
  std::getline(std::cin, cmd);
  if (cmd.empty())
    return;
  std::string out = _client.exec(cmd);

  std::cout << out;
  if (!out.empty() && out.back() != '\n')
    std::cout << std::endl;
}

} // namespace odb
