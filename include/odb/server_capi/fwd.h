//===-- server_capi/fwd.h - Forward declarations ------------------*- C -*-===//
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

#ifndef ODB_SERVER_CAPI_FWD_H_
#define ODB_SERVER_CAPI_FWD_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#include "../server/fwd.hh"
#endif

#define ODB_REG_INFO_NAME_MAX_LENGTH (15)
#define ODB_SYM_INFO_NAME_MAX_LENGTH (127)

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t odb_vm_reg_t;
typedef uint64_t odb_vm_ptr_t;
typedef uint64_t odb_vm_size_t;
typedef int64_t odb_vm_ssize_t;
typedef uint32_t odb_vm_sym_t;

#define ODB_VM_SYM_NULL ((odb_vm_sym_t)(-1))
#define ODB_VM_SYM_ID_NONE ((odb_vm_sym_t)(-1))

typedef enum RegKind {
  ODB_REG_KIND_GENERAL,
  ODB_REG_KIND_PROGRAM_COUNTER,
  ODB_REG_KIND_STACK_POINTER,
  ODB_REG_KIND_BASE_POINTER,
  ODB_REG_KIND_FLAGS,
} odb_reg_kind_t;

typedef enum {
  ODB_RESUME_TYPE_TO_FINISH,
  ODB_RESUME_TYPE_CONTINUE,
  ODB_RESUME_TYPE_STEP,
  ODB_RESUME_TYPE_STEP_OVER,
  ODB_RESUME_TYPE_STEP_OUT,
} odb_resume_type_t;

typedef struct {
  char name[ODB_REG_INFO_NAME_MAX_LENGTH + 1];
  odb_vm_size_t size;
  odb_vm_reg_t idx;
  odb_reg_kind_t kind;
} odb_reg_infos_t;

typedef struct {
  char name[ODB_SYM_INFO_NAME_MAX_LENGTH + 1];
  odb_vm_ptr_t addr;
  odb_vm_sym_t idx;
} odb_symbol_infos_t;

typedef struct {
  const char *name;
  const odb_vm_reg_t *regs_general;
  const odb_vm_reg_t *regs_program_counter;
  const odb_vm_reg_t *regs_stack_pointer;
  const odb_vm_reg_t *regs_base_pointer;
  const odb_vm_reg_t *regs_flags;
  size_t regs_general_size;
  size_t regs_program_counter_size;
  size_t regs_stack_pointer_size;
  size_t regs_base_pointer_size;
  size_t regs_flags_size;
  odb_vm_reg_t regs_count;
  odb_vm_size_t memory_size;
  odb_vm_sym_t symbols_count;
} odb_vm_infos_t;

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace odb {

inline odb_reg_kind_t make_reg_kind(RegKind f) {
  switch (f) {
  case RegKind::general:
    return ODB_REG_KIND_GENERAL;
  case RegKind::program_counter:
    return ODB_REG_KIND_PROGRAM_COUNTER;
  case RegKind::stack_pointer:
    return ODB_REG_KIND_STACK_POINTER;
  case RegKind::base_pointer:
    return ODB_REG_KIND_BASE_POINTER;
  case RegKind::flags:
    return ODB_REG_KIND_FLAGS;
  }
}

inline RegKind make_reg_kind(odb_reg_kind_t f) {
  switch (f) {
  case ODB_REG_KIND_GENERAL:
    return RegKind::general;
  case ODB_REG_KIND_PROGRAM_COUNTER:
    return RegKind::program_counter;
  case ODB_REG_KIND_STACK_POINTER:
    return RegKind::stack_pointer;
  case ODB_REG_KIND_BASE_POINTER:
    return RegKind::base_pointer;
  case ODB_REG_KIND_FLAGS:
    return RegKind::flags;
  default:
    std::terminate();
  }
}

} // namespace odb
#endif

#endif //! ODB_SERVER_CAPI_FWD_H_
