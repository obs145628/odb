#include "odb/mess/tcp-transfer.hh"

#include <unistd.h>

#include "odb/mess/serial.hh"

#include <iostream>

namespace odb {

namespace {

using msg_size_t = std::uint32_t;

bool read_mult(char *out_buf, std::size_t len, int fd) {
  while (len) {
    ssize_t sread = read(fd, out_buf, len);
    if (sread <= 0)
      break;
    std::size_t uread = static_cast<std::size_t>(sread);
    len -= uread;
    out_buf += uread;
  }

  return len == 0;
}

bool write_mult(const char *in_buf, std::size_t len, int fd) {
  while (len) {
    ssize_t swrote = write(fd, in_buf, len);
    if (swrote <= 0)
      break;
    std::size_t uwrote = static_cast<std::size_t>(swrote);
    len -= uwrote;
    in_buf += uwrote;
  }

  return len == 0;
}

} // namespace

bool send_data_tcp(const SerialOutBuff &os, int fd) {

  std::cout << "<< send_data_tcp(" << os.get_size() << ")" << std::endl;

  const char *ptr = os.get_data();
  msg_size_t size = os.get_size();
  auto res =
      write_mult(reinterpret_cast<const char *>(&size), sizeof(size), fd) &&
      write_mult(ptr, size, fd);

  std::cout << "send_data_tcp(" << os.get_size() << ") >>" << std::endl;
  return res;
}

bool recv_data_tcp(SerialInBuff &is, int fd) {

  std::cout << "<< recv_data_tcp()" << std::endl;

  msg_size_t size;
  if (!read_mult(reinterpret_cast<char *>(&size), sizeof(size), fd))
    return false;

  is.reset(size);
  auto res = read_mult(is.get_data(), size, fd);

  std::cout << "recv_data_tcp() >>" << std::endl;
  return res;
}

} // namespace odb
