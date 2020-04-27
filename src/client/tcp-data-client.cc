#include "odb/client/tcp-data-client.hh"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "odb/mess/tcp-transfer.hh"

namespace odb {

namespace {

bool err(const char *msg) {
  std::cerr << "Warning: TCPDataClient: " << msg << "\n";
  return false;
}

} // namespace

TCPDataClient::TCPDataClient(const std::string &hostname, int port)
    : _hostname(hostname), _port(port), _fd(-1) {}

TCPDataClient::~TCPDataClient() {
  if (_fd != -1)
    close(_fd);
}

bool TCPDataClient::connect() {
  if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return err("Failed to create socket");

  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(_port);
  if (inet_pton(AF_INET, _hostname.c_str(), &serv_addr.sin_addr) <= 0)
    return err("Failed to convert hostname to ip");

  if (::connect(_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    return err("Failed to connect to server");

  // Disable Nagle algorithm to solve small packets issue
  int yes = 1;
  if (setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1)
    return err("setsockopt TCP_NODELAY failed");

  return true;
}

bool TCPDataClient::send_data(const SerialOutBuff &os) {
  return send_data_tcp(os, _fd);
}

bool TCPDataClient::recv_data(SerialInBuff &is) {
  return recv_data_tcp(is, _fd);
}

} // namespace odb
