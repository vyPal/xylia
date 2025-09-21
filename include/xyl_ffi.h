#ifndef FFI_H
#define FFI_H

#include <ffi.h>
#include <stdbool.h>

#include "value.h"

typedef enum {
  C_TYPE_VOID,
  C_TYPE_CHAR,
  C_TYPE_SHORT,
  C_TYPE_INT,
  C_TYPE_LONG,
  C_TYPE_UCHAR,
  C_TYPE_USHORT,
  C_TYPE_UINT,
  C_TYPE_ULONG,
  C_TYPE_FLOAT,
  C_TYPE_DOUBLE,
  C_TYPE_PTR,
} c_type_t;

typedef struct c_type_wrapper {
  c_type_t type;
  ffi_type *ffi_t;
  struct c_type_wrapper *points_to;
} c_type_wrapper_t;

typedef struct {
  void *handle;
  char *lib_path;
} c_lib_t;

typedef struct {
  c_lib_t *lib;
  char *symbol_name;
  ffi_cif cif;
  ffi_type **ffi_arg_types;
  c_type_wrapper_t *ret_type;
  c_type_wrapper_t **arg_types;
  void *func_ptr;
  int argc;
} c_func_t;

typedef struct {
  void **ptrs;
  int count;
} marshal_buffer_t;

c_lib_t *clib_load(const char *lib_path);
void clib_unload(c_lib_t *lib);
bool clib_is_loaded(c_lib_t *lib);

c_func_t *cfunc_create(c_lib_t *lib, const char *symbol_name,
                       c_type_wrapper_t *ret_type, c_type_wrapper_t **arg_types,
                       int argc);
void cfunc_free(c_func_t *func);
bool cfunc_is_valid(c_func_t *func);

ffi_type *c_type_to_ffi_type(c_type_t type);

bool marshal_value(value_t *val, c_type_wrapper_t *type, void **out,
                   marshal_buffer_t *buf, bool add_to_buf);

void marshal_buffer_init(marshal_buffer_t *buf);
void marshal_buffer_free(marshal_buffer_t *buf);

void print_cfunc(c_func_t *func);

#endif
