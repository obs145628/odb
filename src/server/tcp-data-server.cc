#include "odb/server/tcp-data-server.hh"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "odb/mess/tcp-transfer.hh"

namespace odb {

namespace {

bool err(const char *msg) {
  std::cerr << "Warning: TCPDataServer: " << msg << "\n";
  return false;
}

} // namespace

TCPDataServer::TCPDataServer(int port)
    : _port(port), _serv_fd(-1), _cli_fd(-1) {}

TCPDataServer::~TCPDataServer() {
  if (_serv_fd != -1)
    close(_serv_fd);
  if (_cli_fd != -1)
    close(_cli_fd);
}

bool TCPDataServer::connect() {
  if ((_serv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return err("Failed to create socket");

  // avoid bind error
  int yes = 1;
  if (setsockopt(_serv_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
    return err("setsockopt failed");

  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(_port);
  if (bind(_serv_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    return err("Bind failed");

  if (listen(_serv_fd, 10) < 0)
    return err("Listen failed !");

  if ((_cli_fd = accept(_serv_fd, (struct sockaddr *)NULL, NULL)) < 0)
    return err("Accept failed");

  return true;
}

bool TCPDataServer::send_data(const SerialOutBuff &os) {
  return send_data_tcp(os, _cli_fd);
}

bool TCPDataServer::recv_data(SerialInBuff &is) {
  return recv_data_tcp(is, _cli_fd);
}

} // namespace odb
