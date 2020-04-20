#include "odb/server/cli-client-handler.hh"

#include <cassert>
#include <cstdio>
#include <iostream>
#include <signal.h>
#include <unistd.h>

#include "odb/server/db-client-impl-vmside.hh"
#include "odb/server/server-app.hh"

namespace odb {

namespace {

void sigint_handler_fn(int) { ServerApp::g_force_stop_db = true; }

} // namespace

CLIClientHandler::CLIClientHandler(Debugger &db, const ServerConfig &conf)
    : ClientHandler(db, conf),
      _db_client(std::make_unique<DBClientImplVMSide>(db)), _client(_db_client),
      _is_tty(isatty(fileno(stdin))), _catch_sigint(false) {}

CLIClientHandler::~CLIClientHandler() { _on_disconnect(); }

void CLIClientHandler::setup_connection() {
  _db_client.connect();
  _client_connected();

  if (get_conf().server_cli_sighandler) {
    // Add signal to stop program execution on Ctrl-C
    struct sigaction sigint_handler;
    sigint_handler.sa_handler = sigint_handler_fn;
    sigemptyset(&sigint_handler.sa_mask);
    sigint_handler.sa_flags = 0;
    sigaction(SIGINT, &sigint_handler, nullptr);
    _catch_sigint = true;
  }
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
  if (!std::cin.good()) {
    _client_disconnected();
    if (_is_tty)
      std::cout << "Debug session closed, program resumed.\n";
    _on_disconnect();
  }

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

void CLIClientHandler::_on_disconnect() {

  if (_catch_sigint) {
    // Remove signal handler
    struct sigaction sigint_handler;
    sigint_handler.sa_handler = SIG_DFL;
    sigemptyset(&sigint_handler.sa_mask);
    sigint_handler.sa_flags = 0;
    sigaction(SIGINT, &sigint_handler, nullptr);
    _catch_sigint = false;
  }
}

} // namespace odb
