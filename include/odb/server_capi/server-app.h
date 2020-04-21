//===-- server_capi/server-app.h - ServerAPI class C wrapper-------*- C -*-===//
//
// ODB Library
// Author: Steven Lariau
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Wrapper around the ServerApp class
/// Used to run the Debugger loop from C code
///
//===----------------------------------------------------------------------===//

#ifndef ODB_SERVER_CAPI_SERVER_APP_H_
#define ODB_SERVER_CAPI_SERVER_APP_H_

#include "vm-api.h"

#ifdef __cplusplus
extern "C" {
#endif

// This is the function responsible to build paremeters needed for VMApi
// returns a pointer to VMAPI table of functions is success, 0 otherwhise
// store in `out_data` the parameter that will be given to every call of the VM
// API
typedef odb_vm_api_vtable_t *(*odb_api_builder_f)(void *arg,
                                                  odb_vm_api_data_t *out_data);

typedef struct {
  void *handle; // opaque C++ handle
} odb_server_app_t;

/// Initialize and allocate memory of `app`
/// 'arg' is the first parameter given to `api_builder`
/// Configuration can be done with environment variables
/// More informations in server/ServerApp.hh
/// api_builder only called if debugger started
void odb_server_app_init(odb_server_app_t *app, odb_api_builder_f api_builder,
                         void *arg);

/// Free all allocated memory and destroy `app`
void odb_server_app_free(odb_server_app_t *app);

/// Enter the debugger loop
void odb_server_app_loop(odb_server_app_t *app);

#ifdef __cplusplus
}
#endif

#endif //! ODB_SERVER_CAPI_SERVER_APP_H_
