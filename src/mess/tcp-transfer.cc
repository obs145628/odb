#include "odb/mess/tcp-transfer.hh"

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "odb/mess/serial.hh"

#include <chrono>
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
    ssize_t swrote = send(fd, in_buf, len, MSG_NOSIGNAL);
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

#ifdef ODB_COMM_LOGS
  auto t1 = std::chrono::high_resolution_clock::now();
  // std::cout << "<< send_data_tcp(" << os.get_size() << ")" << std::endl;
#endif

  const char *ptr = os.get_data();
  msg_size_t size = os.get_size();
  auto res =
      write_mult(reinterpret_cast<const char *>(&size), sizeof(size), fd) &&
      write_mult(ptr, size, fd);

#ifdef ODB_COMM_LOGS
  auto t2 = std::chrono::high_resolution_clock::now();
  auto ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(
                 t1.time_since_epoch())
                 .count();
  auto ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(
                 t2.time_since_epoch())
                 .count();

  auto diff = ms2 - ms1;

  std::cout << "send_data_tcp(" << os.get_size() << ") >>" << ms1 << ", " << ms2
            << ", " << diff << std::endl;
#endif
  return res;
}

bool recv_data_tcp(SerialInBuff &is, int fd) {
#ifdef ODB_COMM_LOGS
  auto t1 = std::chrono::high_resolution_clock::now();
  // std::cout << "<< recv_data_tcp()" << std::endl;
#endif

  msg_size_t size;
  if (!read_mult(reinterpret_cast<char *>(&size), sizeof(size), fd))
    return false;

  is.reset(size);
  auto res = read_mult(is.get_data(), size, fd);

#ifdef ODB_COMM_LOGS
  auto t2 = std::chrono::high_resolution_clock::now();
  auto ms1 = std::chrono::duration_cast<std::chrono::milliseconds>(
                 t1.time_since_epoch())
                 .count();
  auto ms2 = std::chrono::duration_cast<std::chrono::milliseconds>(
                 t2.time_since_epoch())
                 .count();

  auto diff = ms2 - ms1;

  std::cout << "recv_data_tcp(" << size << ") >>" << ms1 << ", " << ms2 << ", "
            << diff << std::endl;
#endif
  return res;
}

} // namespace odb
