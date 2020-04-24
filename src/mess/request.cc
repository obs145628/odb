#include "odb/mess/request.hh"

#include <cstring>

namespace odb {

void ReqInit::build(Message &req) {
  auto &r = req.alloc_as<ReqInit>();
  r.tag = 0;
}

void ReqUpdate::build(Message &req) {
  auto &r = req.alloc_as<ReqUpdate>();
  r.tag = 0;
}

void ReqGetRegs::build(Message &req, const vm_reg_t *ids, vm_size_t reg_size,
                       std::size_t nregs) {
  auto &r = req.alloc_as<ReqGetRegs>(nregs * sizeof(vm_reg_t));
  r.nregs = nregs;
  r.reg_size = reg_size;
  std::memcpy(r.ids(), ids, nregs * sizeof(vm_reg_t));
}

void ReqGetRegsVar::build(Message &req, const vm_reg_t *ids,
                          const vm_size_t *regs_size, std::size_t nregs) {
  auto &r = req.alloc_as<ReqGetRegsVar>(nregs *
                                        (sizeof(vm_reg_t) + sizeof(vm_size_t)));
  r.nregs = nregs;
  std::memcpy(r.ids(), ids, nregs * sizeof(vm_reg_t));
  std::memcpy(r.sizes(), regs_size, nregs * sizeof(vm_size_t));
}

void ReqSetRegs::build(Message &req, const vm_reg_t *ids, const char **in_bufs,
                       vm_size_t reg_size, std::size_t nregs) {
  auto &r = req.alloc_as<ReqSetRegs>(nregs * (sizeof(vm_reg_t) + reg_size));
  r.nregs = nregs;
  r.reg_size = reg_size;
  std::memcpy(r.ids(), ids, nregs * sizeof(vm_reg_t));
  for (std::size_t i = 0; i < nregs; ++i)
    std::memcpy(r.buff(i), in_bufs[i], reg_size);
}

void ReqSetRegsVar::build(Message &req, const vm_reg_t *ids,
                          const char **in_bufs, const vm_size_t *reg_size,
                          std::size_t nregs) {
  std::size_t total_size = nregs * sizeof(vm_reg_t);
  for (std::size_t i = 0; i < nregs; ++i)
    total_size += reg_size[i];

  auto &r = req.alloc_as<ReqSetRegs>(total_size);
  r.nregs = nregs;

  std::memcpy(r.ids(), ids, nregs * sizeof(vm_reg_t));
  char *buff = r.buff();
  for (std::size_t i = 0; i < nregs; ++i) {
    std::memcpy(buff, in_bufs[i], reg_size[i]);
    buff += reg_size[i];
  }
}

void ReqGetRegsInfos::build(Message &req, const vm_reg_t *ids,
                            std::size_t nregs) {
  auto &r = req.alloc_as<ReqGetRegsInfos>(nregs * sizeof(vm_reg_t));
  r.nregs = nregs;
  std::memcpy(r.ids(), ids, nregs * sizeof(vm_reg_t));
}

void ReqReadMem::build(Message &req, const vm_ptr_t *src_addrs,
                       const vm_size_t *bufs_sizes, std::size_t nread) {
  auto &r =
      req.alloc_as<ReqReadMem>(nread * (sizeof(vm_ptr_t) + sizeof(vm_size_t)));
  r.nreads = nread;
  std::memcpy(r.src_addrs(), src_addrs, nread * sizeof(vm_ptr_t));
  std::memcpy(r.bufs_sizes(), bufs_sizes, nread * sizeof(vm_size_t));
}

void ReqWriteMem::build(Message &req, const vm_ptr_t *dst_addrs,
                        const vm_size_t *bufs_sizes, const char **in_bufs,
                        std::size_t nwrites) {
  std::size_t total_size = nwrites * (sizeof(vm_ptr_t) + sizeof(vm_size_t));
  for (std::size_t i = 0; i < nwrites; ++i)
    total_size += bufs_sizes[i];
  auto &r = req.alloc_as<ReqWriteMem>(total_size);

  r.nwrites = nwrites;
  std::memcpy(r.dst_addrs(), dst_addrs, nwrites * sizeof(vm_ptr_t));
  std::memcpy(r.bufs_sizes(), bufs_sizes, nwrites * sizeof(vm_size_t));

  char *buff = r.buff();
  for (std::size_t i = 0; i < nwrites; ++i) {
    std::memcpy(buff, in_bufs[i], bufs_sizes[i]);
    buff += bufs_sizes[i];
  }
}

void ReqGetSymbsById::build(Message &req, const vm_sym_t *ids,
                            std::size_t nsyms) {
  auto &r = req.alloc_as<ReqGetSymbsById>(nsyms * sizeof(vm_sym_t));
  r.nsyms = nsyms;
  std::memcpy(r.ids(), ids, nsyms * sizeof(vm_sym_t));
}

void ReqGetSymbsByAddr::build(Message &req, vm_ptr_t addr, vm_size_t size) {
  auto &r = req.alloc_as<ReqGetSymbsByAddr>();
  r.addr = addr;
  r.size = size;
}

void ReqGetSymbsByNames::build(Message &req, const char **names,
                               std::size_t nsyms) {
  std::size_t total_size = 0;
  for (std::size_t i = 0; i < nsyms; ++i)
    total_size += std::strlen(names[i]) + 1;
  auto &r = req.alloc_as<ReqGetSymbsByNames>(total_size);
  r.nsyms = nsyms;

  char *buff = r.names();
  for (std::size_t i = 0; i < nsyms; ++i) {
    std::size_t len = strlen(names[i]) + 1;
    std::memcpy(buff, names[i], len);
    buff += len;
  }
}

void ReqEditBkps::build(Message &req, const vm_ptr_t *add_addrs,
                        const vm_ptr_t *del_addrs, std::size_t add_count,
                        std::size_t del_count) {
  auto r =
      req.alloc_as<ReqEditBkps>((add_count + del_count) * sizeof(vm_ptr_t));
  r.add_count = add_count;
  r.del_count = del_count;

  std::memcpy(r.add_addrs(), add_addrs, add_count * sizeof(vm_ptr_t));
  std::memcpy(r.del_addrs(), del_addrs, del_count * sizeof(vm_ptr_t));
}

void ReqEditResume::build(Message &req, ResumeType type) {
  auto &r = req.alloc_as<ReqEditResume>();
  switch (type) {
  case ResumeType::ToFinish:
    r.type = 0;
    break;
  case ResumeType::Continue:
    r.type = 1;
    break;
  case ResumeType::Step:
    r.type = 2;
    break;
  case ResumeType::StepOver:
    r.type = 3;
    break;
  case ResumeType::StepOut:
    r.type = 4;
    break;
  };
}

ResumeType ReqEditResume::get_type() {

  ResumeType types[] = {ResumeType::ToFinish, ResumeType::Continue,
                        ResumeType::Step, ResumeType::StepOver,
                        ResumeType::StepOut};
  return types[type];
}

} // namespace odb
