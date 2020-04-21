#include "odb/server_capi/server-app.h"

#include <cassert>

#include "odb/server/server-app.hh"

// extern "C" {

void odb_server_app_init(odb_server_app_t *app, odb_api_builder_f api_builder,
                         void *arg) {
  auto cxx_app = new odb::ServerApp([api_builder, arg]() {
    odb_vm_api_data_t data;
    odb_vm_api_vtable_t *table = api_builder(arg, &data);
    assert(table);
    return odb::make_cpp_vm_api(table, data);
  });
  app->handle = reinterpret_cast<void *>(cxx_app);
}

void odb_server_app_free(odb_server_app_t *app) {
  auto cxx_app = reinterpret_cast<odb::ServerApp *>(app->handle);
  delete cxx_app;
}

void odb_server_app_loop(odb_server_app_t *app) {
  auto cxx_app = reinterpret_cast<odb::ServerApp *>(app->handle);
  cxx_app->loop();
}

//}
