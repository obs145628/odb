#include "odb/server/server-app.hh"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <thread>

#include "odb/server/client-handler.hh"
#include "odb/server/debugger.hh"

#include "odb/mess/db-client-impl.hh"
#include "odb/server/cli-client-handler.hh"

namespace odb {
namespace {
const ServerApp::Config g_conf_default = {
    .enabled = false,
    .nostart = false,
};

constexpr const char *ENV_CONF_ENABLED = "ODB_CONF_ENABLED";
constexpr const char *ENV_CONF_NOSTART = "ODB_CONF_NOSTART";
} // namespace

ServerApp::ServerApp(const Config &conf, const api_builder_f &api_builder)
    : _conf(conf), _api_builder(api_builder) {

  // Load env variables
  auto env_enabled = std::getenv(ENV_CONF_ENABLED);
  if (env_enabled)
    _conf.enabled = std::strcmp(env_enabled, "1") == 0;

  auto env_nostart = std::getenv(ENV_CONF_NOSTART);
  if (env_nostart)
    _conf.nostart = std::strcmp(env_nostart, "1") == 0;
}

ServerApp::ServerApp(const api_builder_f &api_builder)
    : ServerApp(g_conf_default, api_builder) {}

void ServerApp::loop() {
  // Does nothing if debugger disabled
  if (!_conf.enabled)
    return;

  if (_db.get() == nullptr)
    _init();
  else
    _db->on_update();

  // Switching from disconnected to connected stop the program
  if (_client->get_state() == ClientHandler::State::NOT_CONNECTED) {
    _client->setup_connection();
    if (_client->get_state() == ClientHandler::State::CONNECTED)
      _stop_db();
  }

  for (;;) {
    // Returns direcly if client disconnected, or program running
    if (_client->get_state() == ClientHandler::State::DISCONNECTED ||
        !_db_is_stopped())
      break;

    // Block until the client is connected
    _connect();
    assert(_client->get_state() == ClientHandler::State::CONNECTED);

    // Run commands
    _client->run_command();
  }

  // If client disconnected, and DB stopped but not finished, need to resume
  // execution
  if (_client->get_state() == ClientHandler::State::DISCONNECTED &&
      _db->get_state() == Debugger::State::STOPPED)
    _db->resume(ResumeType::ToFinish);
}

void ServerApp::_init() {

  // Create and setup debugger
  _db = std::make_unique<Debugger>(_api_builder());
  auto &db = *_db;
  db.on_init();
  if (_conf.nostart)
    _stop_db();

  // Create and setup client
  // @TODO use a more generic client
  _client = std::make_unique<CLIClientHandler>(db);
}

void ServerApp::_connect() {
  assert(_client->get_state() != ClientHandler::State::DISCONNECTED);
  if (_client->get_state() == ClientHandler::State::CONNECTED)
    return;

  for (;;) {
    _client->setup_connection(); // not-blocking call
    if (_client->get_state() == ClientHandler::State::CONNECTED)
      break;
    std::this_thread::yield(); // sleep before checking again if connected
  }
}

bool ServerApp::_db_is_stopped() {
  auto db_state = _db->get_state();
  return db_state == Debugger::State::STOPPED ||
         db_state == Debugger::State::ERROR ||
         db_state == Debugger::State::EXIT;
}

void ServerApp::_stop_db() {
  if (!_db_is_stopped())
    _db->stop();
}

} // namespace odb
