#include "cli.hh"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <signal.h>
#include <string>
#include <thread>
#include <unistd.h>

#include "odb/client/tcp-data-client.hh"
#include "odb/server/vm-api.hh"

namespace {
bool g_force_stop;

void sigint_handler_fn(int) { g_force_stop = true; }

void prepare_sigint() {
  // Add signal to stop program execution on Ctrl-C
  struct sigaction sigint_handler;
  sigint_handler.sa_handler = sigint_handler_fn;
  sigemptyset(&sigint_handler.sa_mask);
  sigint_handler.sa_flags = 0;
  sigaction(SIGINT, &sigint_handler, nullptr);
}

odb::DBClient build_client(int argc, char **argv) {
  auto hostname = argc >= 2 ? argv[1] : "0.0.0.0";
  auto port = argc >= 3 ? std::atoi(argv[2]) : 12644;

  auto tcp_cli = std::make_unique<odb::TCPDataClient>(hostname, port);
  auto tcp_impl = std::make_unique<odb::DBClientImplData>(std::move(tcp_cli));
  odb::DBClient db_client(std::move(tcp_impl));
  return db_client;
}

} // namespace

CLI::CLI(int argc, char **argv)
    : _db_client(build_client(argc, argv)), _cli(_db_client) {
  _setup();
}

bool CLI::state_switched() {
  auto res = _state_switch;
  _state_switch = false;
  return res;
}

bool CLI::next() {
  auto state = _db_client.state();
  if (state != odb::DBClient::State::VM_STOPPED &&
      state != odb::DBClient::State::VM_RUNNING)
    return false;

  // If VM running, wait for it to stop (need to sends requests at interval
  // to know if still running)
  if (state == odb::DBClient::State::VM_RUNNING) {
    _wait_stop();
    _state_switch = true;
    return next();
  }

  return true;
}

std::string CLI::exec(const std::string &cmd) { return _cli.exec(cmd); }

void CLI::_setup() {
  prepare_sigint();
  _db_client.connect();
  _state_switch = true;
}

void CLI::_wait_stop() {
  g_force_stop = false;

  while (1) {
    try {
      _db_client.check_stopped();
    } catch (odb::VMApi::Error &) {
      // @tip what if I hit Ctrl-r while reading ?
      // I may get only half a message, and corrupt client

      // ignore network errors triggered by Ctrl+C
      if (!g_force_stop) {
        throw;
      }
    }

    auto state = _db_client.state();
    if (state != odb::DBClient::State::VM_RUNNING)
      break;

    if (g_force_stop) {
      _db_client.stop();
      break;
    }

    auto sl_dur = std::chrono::milliseconds(10);
    std::this_thread::sleep_for(sl_dur);
  }
}
