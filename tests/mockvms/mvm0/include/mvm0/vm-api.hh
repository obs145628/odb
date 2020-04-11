#include <odb/server/vm-api.hh>

#pragma once

namespace mvm0 {

class CPU;

class VMApi : public odb::VMApi {

public:
  VMApi(CPU &cpu);

  odb::VMInfos get_vm_infos() override;
  odb::VMApi::UpdateInfos get_update_infos() override;

  void get_reg(odb::vm_reg_t idx, odb::RegInfos &infos, bool val_only) override;
  void set_reg(odb::vm_reg_t idx, const std::uint8_t *new_val) override;
  odb::vm_reg_t find_reg_id(const std::string &name) override;

  void read_mem(odb::vm_ptr_t addr, odb::vm_size_t size,
                std::uint8_t *out_buf) override;
  void write_mem(odb::vm_ptr_t addr, odb::vm_size_t size,
                 const std::uint8_t *buf) override;

  std::vector<odb::vm_sym_t> get_symbols(odb::vm_ptr_t addr,
                                         odb::vm_size_t size) override;
  odb::SymbolInfos get_symb_infos(odb::vm_sym_t idx) override;
  odb::vm_sym_t find_sym_id(const std::string &name) override;

  std::string get_code_text(odb::vm_ptr_t addr,
                            odb::vm_size_t &addr_dist) override;

private:
  CPU &_cpu;
};

} // namespace mvm0
