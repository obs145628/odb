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

template <> void prepare_request(RequestHandler &, ReqStop &) {}

template <> void prepare_request(RequestHandler &h, ReqCheckStopped &r) {
  h.object_out(r.out_udp);
}

template <> void prepare_request(RequestHandler &h, ReqGetRegs &r) {
  h.object_in(r.nregs);
  h.object_in(r.reg_size);
  h.buffer_in(r.ids, r.nregs);
  h.buffer_2d_out(r.out_bufs, r.nregs, r.reg_size);
}

template <> void prepare_request(RequestHandler &h, ReqGetRegsVar &r) {
  h.object_in(r.nregs);
  h.buffer_in(r.in_ids, r.nregs);
  h.buffer_in(r.in_regs_size, r.nregs);
  h.buffer_2dvar_out(r.out_bufs, r.nregs, r.in_regs_size);
}

template <> void prepare_request(RequestHandler &h, ReqSetRegs &r) {
  h.object_in(r.nregs);
  h.object_in(r.reg_size);
  h.buffer_in(r.in_ids, r.nregs);
  h.buffer_2d_in(r.in_bufs, r.nregs, r.reg_size);
}

template <> void prepare_request(RequestHandler &h, ReqSetRegsVar &r) {
  h.object_in(r.nregs);
  h.buffer_in(r.in_ids, r.nregs);
  h.buffer_in(r.in_regs_size, r.nregs);
  h.buffer_2dvar_in(r.in_bufs, r.nregs, r.in_regs_size);
}

template <> void prepare_request(RequestHandler &h, ReqGetRegsInfos &r) {
  h.object_in(r.nregs);
  h.buffer_in(r.ids, r.nregs);
  h.buffer_out(r.out_infos, r.nregs);
}

template <> void prepare_request(RequestHandler &h, ReqFindRegsIds &r) {
  h.object_in(r.nregs);
  h.buffer_2d_in_cstr(r.in_bufs, r.nregs);
  h.buffer_out(r.out_ids, r.nregs);
}

template <> void prepare_request(RequestHandler &h, ReqReadMem &r) {
  h.object_in(r.nbufs);
  h.object_in(r.buf_size);
  h.buffer_in(r.in_addrs, r.nbufs);
  h.buffer_2d_out(r.out_bufs, r.nbufs, r.buf_size);
}

template <> void prepare_request(RequestHandler &h, ReqReadMemVar &r) {
  h.object_in(r.nbufs);
  h.buffer_in(r.in_addrs, r.nbufs);
  h.buffer_in(r.in_bufs_size, r.nbufs);
  h.buffer_2dvar_out(r.out_bufs, r.nbufs, r.in_bufs_size);
}

template <> void prepare_request(RequestHandler &h, ReqWriteMem &r) {
  h.object_in(r.nbufs);
  h.object_in(r.buf_size);
  h.buffer_in(r.in_addrs, r.nbufs);
  h.buffer_2d_in(r.in_bufs, r.nbufs, r.buf_size);
}

template <> void prepare_request(RequestHandler &h, ReqWriteMemVar &r) {
  h.object_in(r.nbufs);
  h.buffer_in(r.in_addrs, r.nbufs);
  h.buffer_in(r.in_bufs_size, r.nbufs);
  h.buffer_2dvar_in(r.in_bufs, r.nbufs, r.in_bufs_size);
}

template <> void prepare_request(RequestHandler &h, ReqGetSymsByIds &r) {
  h.object_in(r.nsyms);
  h.buffer_in(r.in_ids, r.nsyms);
  h.buffer_out(r.out_infos, r.nsyms);
}

template <> void prepare_request(RequestHandler &h, ReqGetSymsByAddr &r) {
  h.object_in(r.addr);
  h.object_in(r.size);
  h.object_out(r.out_infos);
}

template <> void prepare_request(RequestHandler &h, ReqGetSymsByNames &r) {
  h.object_in(r.nsyms);
  h.buffer_2d_in_cstr(r.in_names, r.nsyms);
  h.buffer_out(r.out_infos, r.nsyms);
}

template <> void prepare_request(RequestHandler &h, ReqGetCodeText &r) {
  h.object_in(r.addr);
  h.object_in(r.nins);
  h.object_out(r.out_text);
  h.object_out(r.out_sizes);
}

template <> void prepare_request(RequestHandler &h, ReqAddBkps &r) {
  h.object_in(r.size);
  h.buffer_in(r.in_addrs, r.size);
}

template <> void prepare_request(RequestHandler &h, ReqDelBkps &r) {
  h.object_in(r.size);
  h.buffer_in(r.in_addrs, r.size);
}

template <> void prepare_request(RequestHandler &h, ReqResume &r) {
  h.object_in(r.type);
}

template <> void prepare_request(RequestHandler &h, ReqErr &r) {
  h.object_out(r.msg);
}

RAW_SERIAL(char)
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

template <> void sb_serialize(SerialOutBuff &os, const RegKind &e) {
  sb_serial_raw(os, static_cast<std::int8_t>(e));
}

template <> void sb_unserialize(SerialInBuff &is, RegKind &e) {
  e = static_cast<RegKind>(sb_unserial_raw<std::int8_t>(is));
}

template <> void sb_serialize(SerialOutBuff &os, const RegInfos &reg) {
  os << reg.idx << reg.name << reg.size << reg.kind;
}

template <> void sb_unserialize(SerialInBuff &is, RegInfos &reg) {
  is >> reg.idx >> reg.name >> reg.size >> reg.kind;
}

template <> void sb_serialize(SerialOutBuff &os, const SymbolInfos &sym) {
  os << sym.idx << sym.name << sym.addr;
}

template <> void sb_unserialize(SerialInBuff &is, SymbolInfos &sym) {
  is >> sym.idx >> sym.name >> sym.addr;
}

template <> void sb_serialize(SerialOutBuff &os, const ResumeType &e) {
  sb_serial_raw(os, static_cast<std::int8_t>(e));
}

template <> void sb_unserialize(SerialInBuff &is, ResumeType &e) {
  e = static_cast<ResumeType>(sb_unserial_raw<std::int8_t>(is));
}

} // namespace odb
