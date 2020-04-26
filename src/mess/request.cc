#include "odb/mess/request.hh"

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include "odb/mess/db-client.hh"
#include "odb/mess/request-handler.hh"
#include "odb/server/fwd.hh"

// @xtra I had issues of ambigious call using enable_uf
#define RAW_SERIAL(Ty)                                                         \
  template <> void sb_serialize(SerialOutBuff &os, const Ty &x) {              \
    sb_serial_raw<Ty>(os, x);                                                  \
  }                                                                            \
                                                                               \
  template <> void sb_unserialize(SerialInBuff &is, Ty &x) {                   \
    x = sb_unserial_raw<Ty>(is);                                               \
  }

#if 0
template <class T>
std::enable_if_t<std::is_integral_v<T>> sb_serialize(SerialOutBuff &os,
                                                     const T &x) {
  sb_serial_raw<T>(os, x);
}

template <class T>
std::enable_if_t<std::is_integral_v<T>> sb_unserialize(SerialInBuff &is, T &x) {
  x = sb_unserial_raw<T>(is);
}
#endif

namespace odb {

template <> void prepare_request(RequestHandler &h, ReqConnect &r) {
  h.object_out(r.out_infos);
  h.object_out(r.out_udp);
}

RAW_SERIAL(std::uint8_t)
RAW_SERIAL(std::uint16_t)
RAW_SERIAL(std::uint32_t)
RAW_SERIAL(std::uint64_t)
RAW_SERIAL(std::int8_t)
RAW_SERIAL(std::int16_t)
RAW_SERIAL(std::int32_t)
RAW_SERIAL(std::int64_t)

template <> void sb_serialize(SerialOutBuff &os, const ReqType &ty) {
  sb_serial_raw(os, static_cast<std::int8_t>(ty));
}

template <> void sb_unserialize(SerialInBuff &is, ReqType &ty) {
  ty = static_cast<ReqType>(sb_unserial_raw<std::int8_t>(is));
}

template <> void sb_serialize(SerialOutBuff &os, const std::string &s) {
  sb_serial_raw<std::uint64_t>(os, s.size());
  os.write(s.c_str(), s.size());
}

template <> void sb_unserialize(SerialInBuff &is, std::string &s) {
  auto size = sb_unserial_raw<std::uint64_t>(is);
  s.resize(size);
  is.read(&s[0], size);
}

template <class T>
void sb_serialize(SerialOutBuff &os, const std::vector<T> &v) {
  sb_serial_raw<std::uint64_t>(os, v.size());
  for (std::size_t i = 0; i < v.size(); ++i)
    os << v[i];
}

// @extra doesn't work if T have no default constructor
template <class T> void sb_unserialize(SerialInBuff &is, std::vector<T> &v) {
  auto size = sb_unserial_raw<std::uint64_t>(is);
  v.resize(size);
  for (std::size_t i = 0; i < v.size(); ++i)
    is >> v[i];
}

template <> void sb_serialize(SerialOutBuff &os, const VMInfos &infos) {
  os << infos.name << infos.regs_count << infos.regs_general
     << infos.regs_program_counter << infos.regs_stack_pointer
     << infos.regs_base_pointer << infos.regs_flags << infos.memory_size
     << infos.symbols_count << infos.pointer_size << infos.integer_size;

  sb_serial_raw<std::uint8_t>(os, infos.use_opcode);
}

template <> void sb_unserialize(SerialInBuff &is, VMInfos &infos) {
  is >> infos.name >> infos.regs_count >> infos.regs_general >>
      infos.regs_program_counter >> infos.regs_stack_pointer >>
      infos.regs_base_pointer >> infos.regs_flags >> infos.memory_size >>
      infos.symbols_count >> infos.pointer_size >> infos.integer_size;

  infos.use_opcode = sb_unserial_raw<std::uint8_t>(is);
}

template <> void sb_serialize(SerialOutBuff &os, const StoppedState &e) {
  sb_serial_raw(os, static_cast<std::int8_t>(e));
}

template <> void sb_unserialize(SerialInBuff &is, StoppedState &e) {
  e = static_cast<StoppedState>(sb_unserial_raw<std::int8_t>(is));
}

template <> void sb_serialize(SerialOutBuff &os, const CallInfos &ci) {
  os << ci.caller_start_addr << ci.call_addr;
}

template <> void sb_unserialize(SerialInBuff &is, CallInfos &ci) {
  is >> ci.caller_start_addr >> ci.call_addr;
}

template <> void sb_serialize(SerialOutBuff &os, const DBClientUpdate &udp) {
  os << udp.vm_state;
  sb_serial_raw<std::uint8_t>(os, udp.stopped);
  os << udp.addr;
  os << udp.stack;
}

template <> void sb_unserialize(SerialInBuff &is, DBClientUpdate &udp) {
  is >> udp.vm_state;
  udp.stopped = sb_unserial_raw<std::uint8_t>(is);
  is >> udp.addr;
  is >> udp.stack;
}

} // namespace odb
