//===-- server_capi/vm-api.h - VMApi class C wrapper---------------*- C -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Wrapper around the VMAPI abstract interface
/// Use a table of function pointers and a data argument
///
//===----------------------------------------------------------------------===//

#ifndef ODB_SERVER_CAPI_VM_API_H_
#define ODB_SERVER_CAPI_VM_API_H_

#include "fwd.h"

#ifdef __cplusplus
#include "../server/vm-api.hh"
#include <memory>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ODB_VM_API_ERR_MSG_MAX_LENGTH (255)
#define ODB_VM_API_SYMS_LIST_CAP (32)
#define ODB_VM_API_TEXT_INS_CAP (64)

typedef enum {
  ODB_VM_API_UPDATE_STATE_ERROR,
  ODB_VM_API_UPDATE_STATE_EXIT,
  ODB_VM_API_UPDATE_STATE_CALL_SUB,
  ODB_VM_API_UPDATE_STATE_RET_SUB,
  ODB_VM_API_UPDATE_STATE_OK,
} odb_vm_api_update_state_t;

typedef struct {
  odb_vm_api_update_state_t state;
  odb_vm_ptr_t act_addr;
} odb_vm_api_update_infos_t;

typedef struct {
  char msg[ODB_VM_API_ERR_MSG_MAX_LENGTH];
} odb_vm_api_error_t;

typedef void *odb_vm_api_data_t;

// C functions for the abstract members of odb::VMApi
// All functions have `data` and `err` param
// `data`: argument given during object construction
// `err`: initialized at empty str, means no error.
// Must be overwritten to generate C++ exceptions

// Must store result in out_infos
// All const pointers refers must be always valid, and content must not change
// Should be rodata objects
typedef void (*odb_vm_api_get_vm_infos_f)(odb_vm_api_data_t data,
                                          odb_vm_api_error_t *err,
                                          odb_vm_infos_t *out_infos);

// Must store result in `out_udp`
typedef void (*odb_vm_api_get_update_infos_f)(
    odb_vm_api_data_t data, odb_vm_api_error_t *err,
    odb_vm_api_update_infos_t *out_udp);

// Must store result in `out_reg`
typedef void (*odb_vm_api_get_reg_infos_f)(odb_vm_api_data_t data,
                                           odb_vm_api_error_t *err,
                                           odb_vm_reg_t idx,
                                           odb_reg_infos_t *out_reg);

// Must store result in `out_buf`
typedef void (*odb_vm_api_get_reg_val_f)(odb_vm_api_data_t data,
                                         odb_vm_api_error_t *err,
                                         odb_vm_reg_t idx, void *out_buf);

typedef void (*odb_vm_api_set_reg_f)(odb_vm_api_data_t data,
                                     odb_vm_api_error_t *err, odb_vm_reg_t idx,
                                     const void *buf);

typedef odb_vm_reg_t (*odb_vm_api_find_reg_id_f)(odb_vm_api_data_t data,
                                                 odb_vm_api_error_t *err,
                                                 const char *name);

// Must store result in `out_buf`
typedef void (*odb_vm_api_read_mem_f)(odb_vm_api_data_t data,
                                      odb_vm_api_error_t *err,
                                      odb_vm_ptr_t addr, odb_vm_size_t size,
                                      void *out_buf);

typedef void (*odb_vm_api_write_mem_f)(odb_vm_api_data_t data,
                                       odb_vm_api_error_t *err,
                                       odb_vm_ptr_t addr, odb_vm_size_t size,
                                       const void *buf);

// Store result in `out_syms,
// cannot store more than `ODB_VM_API_SYMS_LIST_CAP` indices
// Store number of symbols in `out_nb_syms`
// Store in `out_act_size` the number of addresses visited before reaching the
// array limit, or `size` if limit not reached
// When limit reached, other calls are done to this function until all ids are
// loaded
typedef void (*odb_vm_api_get_symbols_f)(odb_vm_api_data_t data,
                                         odb_vm_api_error_t *err,
                                         odb_vm_ptr_t addr, odb_vm_size_t size,
                                         odb_vm_sym_t *out_syms,
                                         size_t *out_nb_syms,
                                         odb_vm_size_t *out_act_size);

// Store result in `out_sym`
typedef void (*odb_vm_api_get_symb_infos_f)(odb_vm_api_data_t data,
                                            odb_vm_api_error_t *err,
                                            odb_vm_sym_t idx,
                                            odb_symbol_infos_t *out_sym);

typedef odb_vm_sym_t (*odb_vm_api_find_sym_id_f)(odb_vm_api_data_t data,
                                                 odb_vm_api_error_t *err,
                                                 const char *name);

// Store result in `out_text`
//   must not store more than `ODB_VM_API_TEXT_INS_CAP` bytes, '\0' included
// Store addr offset in `out_addr_dist`
typedef void (*odb_vm_api_get_code_text_f)(odb_vm_api_data_t data,
                                           odb_vm_api_error_t *err,
                                           odb_vm_ptr_t addr, char *out_text,
                                           odb_vm_size_t *out_addr_dist);

// Called when the VMApi object is destroyed
// To release all used ressources
typedef void (*odb_vm_api_cleanup_f)(odb_vm_api_data_t data);

typedef struct {
  odb_vm_api_get_vm_infos_f get_vm_infos;
  odb_vm_api_get_update_infos_f get_update_infos;
  odb_vm_api_get_reg_infos_f get_reg_infos;
  odb_vm_api_get_reg_val_f get_reg_val;
  odb_vm_api_set_reg_f set_reg;
  odb_vm_api_find_reg_id_f find_reg_id;
  odb_vm_api_read_mem_f read_mem;
  odb_vm_api_write_mem_f write_mem;
  odb_vm_api_get_symbols_f get_symbols;
  odb_vm_api_get_symb_infos_f get_symb_infos;
  odb_vm_api_find_sym_id_f find_sym_id;
  odb_vm_api_get_code_text_f get_code_text;
  odb_vm_api_cleanup_f cleanup;
} odb_vm_api_vtable_t;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

namespace odb {
class VMApi;

std::unique_ptr<VMApi> make_cpp_vm_api(odb_vm_api_vtable_t *table,
                                       odb_vm_api_data_t data);

#if 0
typedef enum {
  ODB_VM_API_UPDATE_STATE_ERROR,
  ODB_VM_API_UPDATE_STATE_EXIT,
  ODB_VM_API_UPDATE_STATE_CALL_SUB,
  ODB_VM_API_UPDATE_STATE_RET_SUB,
  ODB_VM_API_UPDATE_STATE_OK,
} odb_vm_api_update_state_t;
#endif

inline odb_vm_api_update_state_t
make_vm_api_update_state(VMApi::UpdateState f) {
  switch (f) {
  case VMApi::UpdateState::ERROR:
    return ODB_VM_API_UPDATE_STATE_ERROR;
  case VMApi::UpdateState::EXIT:
    return ODB_VM_API_UPDATE_STATE_EXIT;
  case VMApi::UpdateState::CALL_SUB:
    return ODB_VM_API_UPDATE_STATE_CALL_SUB;
  case VMApi::UpdateState::RET_SUB:
    return ODB_VM_API_UPDATE_STATE_RET_SUB;
  case VMApi::UpdateState::OK:
    return ODB_VM_API_UPDATE_STATE_OK;
  }
}

inline VMApi::UpdateState
make_vm_api_update_state(odb_vm_api_update_state_t f) {
  switch (f) {
  case ODB_VM_API_UPDATE_STATE_ERROR:
    return VMApi::UpdateState::ERROR;
  case ODB_VM_API_UPDATE_STATE_EXIT:
    return VMApi::UpdateState::EXIT;
  case ODB_VM_API_UPDATE_STATE_CALL_SUB:
    return VMApi::UpdateState::CALL_SUB;
  case ODB_VM_API_UPDATE_STATE_RET_SUB:
    return VMApi::UpdateState::RET_SUB;
  case ODB_VM_API_UPDATE_STATE_OK:
    return VMApi::UpdateState::OK;
  default:
    std::terminate();
  }
}

} // namespace odb

#endif

#endif //! ODB_SERVER_CAPI_VM_API_H_
