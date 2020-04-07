#include "odb/server/client-handler.hh"

namespace odb {

ClientHandler::ClientHandler(Debugger &debugger)
    : _debugger(debugger), _state(State::NOT_CONNECTED) {}

void ClientHandler::_client_connected() { _state = State::CONNECTED; }

void ClientHandler::_client_disconnected() { _state = State::DISCONNECTED; }

} // namespace odb
