#include "tcp-data-client.hh"

namespace odb {

TCPDataClient(const std::string &hostname, int port)
    : _hostname(hostname), _port(port), _fd(-1) {}

} // namespace odb
