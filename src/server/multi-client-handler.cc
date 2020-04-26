#include "odb/server/multi-client-handler.hh"

#include <cassert>

#include "odb/server/cli-client-handler.hh"
#include "odb/server/data-client-handler.hh"
#include "odb/server/server-app.hh"

namespace odb {

MultiClientHandler::MultiClientHandler(Debugger &db, const ServerConfig &conf)
    : ClientHandler(db, conf) {

  if (conf.mode_server_cli)
    _wait.push_back(std::make_unique<CLIClientHandler>(db, conf));
  if (conf.mode_tcp)
    _wait.push_back(std::make_unique<DataClientHandler>(
        db, conf, DataClientHandler::Kind::TCP_SERVER));
}

void MultiClientHandler::setup_connection() {
  assert(_main.get() == nullptr);

  for (auto &cli : _wait) {
    cli->setup_connection();
    if (cli->get_state() == State::CONNECTED) {
      // make cli the main client and clear list
      _client_connected();
      _main = std::move(cli);
      _wait.clear();
      return;
    }
  }
}

void MultiClientHandler::run_command() {
  assert(_main.get() != nullptr);

  _main->run_command();
  if (_main->get_state() == State::DISCONNECTED)
    _client_disconnected();
}

} // namespace odb
