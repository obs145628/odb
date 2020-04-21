#include "odb/server_capi/vm-api.h"
#include "odb/server/vm-api.hh"

#include <cstring>

namespace odb {

namespace {

class CWrapperVMApi : public VMApi {

public:
  CWrapperVMApi(odb_vm_api_vtable_t *table, odb_vm_api_data_t data)
      : _table(table), _data(data) {}

  ~CWrapperVMApi() override { _table->cleanup(_data); }

  VMInfos get_vm_infos() override {
    VMInfos cpp_infos;
    odb_vm_infos_t c_infos;
    _err.msg[0] = 0;
    _table->get_vm_infos(_data, &_err, &c_infos);
    if (_err.msg[0])
      throw VMApi::Error(_err.msg);

    // Convert C to C++ struct
    cpp_infos.name = c_infos.name;
    cpp_infos.regs_count = c_infos.regs_count;
    cpp_infos.regs_general.assign(
        c_infos.regs_general, c_infos.regs_general + c_infos.regs_general_size);
    cpp_infos.regs_program_counter.assign(
        c_infos.regs_program_counter,
        c_infos.regs_program_counter + c_infos.regs_program_counter_size);
    cpp_infos.regs_stack_pointer.assign(c_infos.regs_stack_pointer,
                                        c_infos.regs_stack_pointer +
                                            c_infos.regs_stack_pointer_size);
    cpp_infos.regs_base_pointer.assign(c_infos.regs_base_pointer,
                                       c_infos.regs_base_pointer +
                                           c_infos.regs_base_pointer_size);
    cpp_infos.regs_flags.assign(c_infos.regs_flags,
                                c_infos.regs_flags + c_infos.regs_flags_size);
    cpp_infos.memory_size = c_infos.memory_size;
    cpp_infos.symbols_count = c_infos.symbols_count;
    cpp_infos.pointer_size = c_infos.pointer_size;
    cpp_infos.integer_size = c_infos.integer_size;
    cpp_infos.use_opcode = c_infos.use_opcode;

    return cpp_infos;
  }

  VMApi::UpdateInfos get_update_infos() override {
    VMApi::UpdateInfos cpp_upd;
    odb_vm_api_update_infos_t c_upd;
    _err.msg[0] = 0;
    _table->get_update_infos(_data, &_err, &c_upd);
    if (_err.msg[0])
      throw VMApi::Error(_err.msg);

    cpp_upd.state = make_vm_api_update_state(c_upd.state);
    cpp_upd.act_addr = c_upd.act_addr;
    return cpp_upd;
  }

  void get_reg(vm_reg_t idx, RegInfos &infos, bool val_only) override {
    odb_reg_infos_t c_reg;
    _err.msg[0] = 0;
    if (val_only)
      _table->get_reg_val(_data, &_err, idx, &infos.val[0]);
    else
      _table->get_reg_infos(_data, &_err, idx, &c_reg);
    if (_err.msg[0])
      throw VMApi::Error(_err.msg);

    if (val_only)
      return;

    infos.idx = c_reg.idx;
    infos.name = c_reg.name;
    infos.size = c_reg.size;
    infos.kind = make_reg_kind(c_reg.kind);
  }

  void set_reg(vm_reg_t idx, const std::uint8_t *new_val) override {
    _err.msg[0] = 0;
    _table->set_reg(_data, &_err, idx, new_val);
    if (_err.msg[0])
      throw VMApi::Error(_err.msg);
  }

  vm_reg_t find_reg_id(const std::string &name) override {
    _err.msg[0] = 0;
    auto res = _table->find_reg_id(_data, &_err, name.c_str());
    if (_err.msg[0])
      throw VMApi::Error(_err.msg);
    return res;
  }

  void read_mem(vm_ptr_t addr, vm_size_t size, std::uint8_t *out_buf) override {
    _err.msg[0] = 0;
    _table->read_mem(_data, &_err, addr, size, out_buf);
    if (_err.msg[0])
      throw VMApi::Error(_err.msg);
  }

  void write_mem(vm_ptr_t addr, vm_size_t size,
                 const std::uint8_t *buf) override {
    _err.msg[0] = 0;
    _table->write_mem(_data, &_err, addr, size, buf);
    if (_err.msg[0])
      throw VMApi::Error(_err.msg);
  }

  std::vector<vm_sym_t> get_symbols(vm_ptr_t addr, vm_size_t size) override {
    std::vector<vm_sym_t> res;
    vm_sym_t syms_buf[ODB_VM_API_SYMS_LIST_CAP];
    size_t syms_buf_size;
    vm_size_t act_size;
    _err.msg[0] = 0;

    while (size) {
      _table->get_symbols(_data, &_err, addr, size, syms_buf, &syms_buf_size,
                          &act_size);
      if (_err.msg[0])
        throw VMApi::Error(_err.msg);
      size -= act_size;
      addr += act_size;
      res.insert(res.end(), &syms_buf[0], &syms_buf[0] + syms_buf_size);
    }

    return res;
  }

  SymbolInfos get_symb_infos(vm_sym_t idx) override {
    SymbolInfos cpp_sym;
    odb_symbol_infos_t c_sym;
    _err.msg[0] = 0;
    _table->get_symb_infos(_data, &_err, idx, &c_sym);
    if (_err.msg[0])
      throw VMApi::Error(_err.msg);

    cpp_sym.idx = c_sym.idx;
    cpp_sym.name = c_sym.name;
    cpp_sym.addr = c_sym.addr;
    return cpp_sym;
  }

  vm_sym_t find_sym_id(const std::string &name) override {
    _err.msg[0] = 0;
    auto res = _table->find_sym_id(_data, &_err, name.c_str());
    if (_err.msg[0])
      throw VMApi::Error(_err.msg);
    return res;
  }

  std::string get_code_text(vm_ptr_t addr, vm_size_t &addr_dist) override {
    std::string res;
    res.resize(ODB_VM_API_TEXT_INS_CAP);
    _err.msg[0] = 0;
    _table->get_code_text(_data, &_err, addr, &res[0], &addr_dist);
    if (_err.msg[0])
      throw VMApi::Error(_err.msg);

    res.resize(strlen(res.c_str()));
    return res;
  }

private:
  odb_vm_api_vtable_t *_table;
  odb_vm_api_data_t _data;
  odb_vm_api_error_t _err;
};

} // namespace

std::unique_ptr<VMApi> make_cpp_vm_api(odb_vm_api_vtable_t *table,
                                       odb_vm_api_data_t data) {
  return std::make_unique<CWrapperVMApi>(table, data);
}

} // namespace odb
