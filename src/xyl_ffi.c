#include <dlfcn.h>
#include <ffi.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "value.h"
#include "xyl_ffi.h"

c_lib_t *clib_load(const char *lib_path) {
  if (!lib_path)
    return NULL;

  c_lib_t *lib = malloc(sizeof(c_lib_t));
  if (!lib)
    return NULL;

  lib->handle = dlopen(lib_path, RTLD_LAZY);
  if (!lib->handle) {
    fprintf(stderr, "Failed to open library %s: %s\n", lib_path, dlerror());
    free(lib);
    return NULL;
  }

  lib->lib_path = strdup(lib_path);
  return lib;
}

void clib_unload(c_lib_t *lib) {
  if (!lib)
    return;

  if (lib->handle)
    dlclose(lib->handle);
  if (lib->lib_path)
    free(lib->lib_path);
  free(lib);
}

bool clib_is_loaded(c_lib_t *lib) {
  return lib && lib->handle;
}

c_func_t *cfunc_create(c_lib_t *lib, const char *symbol_name,
                       c_type_wrapper_t *ret_type, c_type_wrapper_t **arg_types,
                       int argc) {
  if (!lib || !symbol_name || !clib_is_loaded(lib))
    return NULL;

  c_func_t *func = malloc(sizeof(c_func_t));
  if (!func)
    return NULL;

  func->func_ptr = dlsym(lib->handle, symbol_name);
  if (!func->func_ptr) {
    fprintf(stderr, "Symbol %s not found: %s\n", symbol_name, dlerror());
    free(func);
    return NULL;
  }

  func->lib = lib;
  func->symbol_name = strdup(symbol_name);
  func->argc = argc;
  func->ret_type = ret_type;

  if (argc > 0)
    func->arg_types = arg_types;
  else
    func->arg_types = NULL;

  ffi_type **ffi_arg_types = NULL;
  if (argc > 0) {
    ffi_arg_types = (ffi_type **)malloc(sizeof(ffi_type *) * argc);
    if (!ffi_arg_types) {
      fprintf(stderr, "Failed to allocate ffi_arg_types\n");
      if (func->arg_types)
        free(func->arg_types);
      free(func->symbol_name);
      free(func);
      return NULL;
    }
    for (int i = 0; i < argc; i++)
      ffi_arg_types[i] = func->arg_types[i]->ffi_t;
  }

  func->ffi_arg_types = ffi_arg_types;

  if (ffi_prep_cif(&func->cif, FFI_DEFAULT_ABI, argc, func->ret_type->ffi_t,
                   ffi_arg_types) != FFI_OK) {
    fprintf(stderr, "Failed to prepare CIF for %s\n", symbol_name);
    free(ffi_arg_types);
    if (func->arg_types)
      free(func->arg_types);
    free(func->symbol_name);
    free(func);
    return NULL;
  }

  return func;
}

static void c_type_wrapper_free(c_type_wrapper_t *type) {
  if (!type)
    return;

  if (type->points_to) {
    c_type_wrapper_free(type->points_to);
    type->points_to = NULL;
  }

  free(type);
}

void cfunc_free(c_func_t *func) {
  if (!func)
    return;

  if (func->ffi_arg_types) {
    free(func->ffi_arg_types);
    func->ffi_arg_types = NULL;
  }

  if (func->symbol_name) {
    free(func->symbol_name);
    func->symbol_name = NULL;
  }

  if (func->arg_types) {
    for (int i = 0; i < func->argc; i++)
      c_type_wrapper_free(func->arg_types[i]);
    free(func->arg_types);
    func->arg_types = NULL;
  }

  c_type_wrapper_free(func->ret_type);

  free(func);
}

bool cfunc_is_valid(c_func_t *func) {
  return func && func->func_ptr && func->lib && clib_is_loaded(func->lib);
}

ffi_type *c_type_to_ffi_type(c_type_t type) {
  switch (type) {
  case C_TYPE_VOID:
    return &ffi_type_void;
  case C_TYPE_CHAR:
    return &ffi_type_sint8;
  case C_TYPE_SHORT:
    return &ffi_type_sint16;
  case C_TYPE_INT:
    return &ffi_type_sint32;
  case C_TYPE_LONG:
    return &ffi_type_sint64;
  case C_TYPE_UCHAR:
    return &ffi_type_uint8;
  case C_TYPE_USHORT:
    return &ffi_type_uint16;
  case C_TYPE_UINT:
    return &ffi_type_uint32;
  case C_TYPE_ULONG:
    return &ffi_type_uint64;
  case C_TYPE_FLOAT:
    return &ffi_type_float;
  case C_TYPE_DOUBLE:
    return &ffi_type_double;
  case C_TYPE_PTR:
    return &ffi_type_pointer;
  }
}

bool marshal_value(value_t *val, c_type_wrapper_t *type, void **out,
                   marshal_buffer_t *buf, bool add_to_buf) {
  if (!type)
    return false;

  if (type->type == C_TYPE_PTR) {
    if (IS_REF(*val)) {
      obj_ref_t *ref = (obj_ref_t *)val->as.obj;

      void *inner_ptr;
      if (!marshal_value(&ref->val, type->points_to, &inner_ptr, buf, false))
        return false;

      // TODO: marshal all values by ref
      switch (type->points_to->type) {
      case C_TYPE_CHAR:
      case C_TYPE_SHORT:
      case C_TYPE_INT:
      case C_TYPE_LONG:
      case C_TYPE_UCHAR:
      case C_TYPE_USHORT:
      case C_TYPE_UINT:
      case C_TYPE_ULONG:
      case C_TYPE_FLOAT:
      case C_TYPE_DOUBLE:
      case C_TYPE_PTR:
      case C_TYPE_VOID:
        break;
      }

      void **ptr_storage = malloc(sizeof(void *));
      if (!ptr_storage)
        return false;

      *ptr_storage = inner_ptr;

      buf->ptrs = realloc(buf->ptrs, sizeof(void *) * (buf->count + 1));
      buf->ptrs[buf->count++] = ptr_storage;

      *out = ptr_storage;
      return true;
    } else if (IS_NIL(*val)) {
      *out = NULL;
      return true;
    } else if (IS_STRING(*val)) {
      *out = AS_STRING(*val)->chars;
      return true;
    }
  } else {
    void *tmp = malloc(type->ffi_t->size);
    if (!tmp)
      return false;

    switch (type->type) {
    case C_TYPE_CHAR: {
      if (IS_NUMBER(*val) && INT8_MIN <= AS_NUMBER(*val) &&
          AS_NUMBER(*val) <= INT8_MAX) {
        *(int8_t *)tmp = (int8_t)AS_NUMBER(*val);
      } else if (IS_STRING(*val)) {
        obj_string_t *str = AS_STRING(*val);
        if (str->length != 1) {
          free(tmp);
          return false;
        }
        *(int8_t *)tmp = str->chars[0];
      } else {
        free(tmp);
        return false;
      }
    } break;

    case C_TYPE_SHORT: {
      if (IS_NUMBER(*val) && INT16_MIN <= AS_NUMBER(*val) &&
          AS_NUMBER(*val) <= INT16_MAX) {
        *(int16_t *)tmp = (int16_t)AS_NUMBER(*val);
      } else {
        free(tmp);
        return false;
      }
    } break;

    case C_TYPE_INT: {
      if (IS_NUMBER(*val) && INT32_MIN <= AS_NUMBER(*val) &&
          AS_NUMBER(*val) <= INT32_MAX) {
        *(int32_t *)tmp = (int32_t)AS_NUMBER(*val);
      } else {
        free(tmp);
        return false;
      }
    } break;

    case C_TYPE_LONG: {
      if (!IS_NUMBER(*val)) {
        free(tmp);
        return false;
      }
      *(int64_t *)tmp = AS_NUMBER(*val);
    } break;

    case C_TYPE_UCHAR: {
      if (IS_NUMBER(*val) && 0 <= AS_NUMBER(*val) &&
          AS_NUMBER(*val) <= UINT8_MAX) {
        *(uint8_t *)tmp = (uint8_t)AS_NUMBER(*val);
      } else {
        free(tmp);
        return false;
      }
    } break;

    case C_TYPE_USHORT: {
      if (IS_NUMBER(*val) && 0 <= AS_NUMBER(*val) &&
          AS_NUMBER(*val) <= UINT16_MAX) {
        *(uint16_t *)tmp = (uint16_t)AS_NUMBER(*val);
      } else {
        free(tmp);
        return false;
      }
    } break;

    case C_TYPE_UINT: {
      if (IS_NUMBER(*val) && 0 <= AS_NUMBER(*val) &&
          AS_NUMBER(*val) <= UINT32_MAX) {
        *(uint32_t *)tmp = (uint32_t)AS_NUMBER(*val);
      } else {
        free(tmp);
        return false;
      }
    } break;

    case C_TYPE_ULONG: {
      if (IS_NUMBER(*val) && 0 <= AS_NUMBER(*val)) {
        *(uint64_t *)tmp = (uint64_t)AS_NUMBER(*val);
      } else {
        free(tmp);
        return false;
      }
    } break;

    case C_TYPE_FLOAT: {
      if (IS_FLOAT(*val) && !isnan(AS_FLOAT(*val)) && !isinf(AS_FLOAT(*val)) &&
          -FLT_MAX <= AS_FLOAT(*val) && AS_FLOAT(*val) <= FLT_MAX) {
        *(float *)tmp = (float)AS_FLOAT(*val);
      } else {
        free(tmp);
        return false;
      }
    } break;

    case C_TYPE_DOUBLE: {
      if (!IS_FLOAT(*val)) {
        free(tmp);
        return false;
      }
      *(double *)tmp = AS_FLOAT(*val);
    } break;

    default:
      free(tmp);
      return false;
    }

    if (add_to_buf) {
      buf->ptrs = realloc(buf->ptrs, sizeof(void *) * (buf->count + 1));
      buf->ptrs[buf->count++] = tmp;
    }

    *out = tmp;
    return true;
  }

  return false;
}

void marshal_buffer_init(marshal_buffer_t *buf) {
  buf->ptrs = NULL;
  buf->count = 0;
}

void marshal_buffer_free(marshal_buffer_t *buf) {
  for (int i = 0; i < buf->count; i++)
    free(buf->ptrs[i]);
  free(buf->ptrs);
  buf->ptrs = NULL;
  buf->count = 0;
}

static void print_c_type(c_type_wrapper_t *type) {
  if (type->type == C_TYPE_PTR) {
    print_c_type(type->points_to);
    printf("*");
  } else {
    switch (type->type) {
    case C_TYPE_VOID:
      printf("void");
      break;
    case C_TYPE_CHAR:
      printf("char");
      break;
    case C_TYPE_SHORT:
      printf("short");
      break;
    case C_TYPE_INT:
      printf("int");
      break;
    case C_TYPE_LONG:
      printf("long");
      break;
    case C_TYPE_UCHAR:
      printf("unsigned char");
      break;
    case C_TYPE_USHORT:
      printf("unsigned short");
      break;
    case C_TYPE_UINT:
      printf("unsigned int");
      break;
    case C_TYPE_ULONG:
      printf("unsigned long");
      break;
    case C_TYPE_FLOAT:
      printf("float");
      break;
    case C_TYPE_DOUBLE:
      printf("double");
      break;
    case C_TYPE_PTR:
      break;
    }
  }
}

void print_cfunc(c_func_t *func) {
  print_c_type(func->ret_type);
  printf(" %s(", func->symbol_name);
  for (int i = 0; i < func->argc; i++) {
    if (i != 0)
      printf(", ");
    print_c_type(func->arg_types[i]);
  }
  printf(");");
}
