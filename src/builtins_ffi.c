#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"
#include "xyl_ffi.h"

xyl_builtin(dll_open) {
  xyl_builtin_signature(dll_open, 1, ARGC_EXACT, {VAL_OBJ, OBJ_STRING});
  obj_string_t *dll_path = AS_STRING(argv[0]);

  c_lib_t *c_lib = clib_load(dll_path->chars);
  if (!c_lib) {
    runtime_error(-1, "Failed to load c lib '%s'", dll_path->chars);
    return NIL_VAL;
  }

  obj_c_lib_t *obj_c_lib = new_c_lib(c_lib);
  if (!obj_c_lib) {
    runtime_error(-1, "Failed to create obj_c_lib_t");
    return NIL_VAL;
  }

  return OBJ_VAL(obj_c_lib);
}

static bool value_to_c_type_wrapper(value_t val, c_type_wrapper_t *out) {
  if (IS_NUMBER(val)) {
    if (AS_NUMBER(val) < C_TYPE_VOID || AS_NUMBER(val) > C_TYPE_DOUBLE)
      return false;
    out->type = (c_type_t)AS_NUMBER(val);
    out->ffi_t = c_type_to_ffi_type(out->type);
    out->points_to = NULL;
    return true;
  } else if (IS_INSTANCE(val)) {
    obj_instance_t *inst = AS_INSTANCE(val);
    if (strcmp(inst->clas->name->chars, "PTR") != 0)
      return false;

    value_t inner_type_val;
    if (!table_get(&inst->fields, copy_string("__type", 6, true),
                   &inner_type_val))
      return false;

    out->points_to = malloc(sizeof(c_type_wrapper_t));
    if (!out->points_to)
      return false;

    if (!value_to_c_type_wrapper(inner_type_val, out->points_to)) {
      free(out->points_to);
      return false;
    }

    out->type = C_TYPE_PTR;
    out->ffi_t = &ffi_type_pointer;
    return true;
  }

  return false;
}

xyl_builtin(dll_function) {
  xyl_builtin_signature(dll_function, 4, ARGC_EXACT, {VAL_OBJ, OBJ_C_LIB},
                        {VAL_OBJ, OBJ_STRING}, {VAL_ANY, OBJ_ANY},
                        {VAL_OBJ, OBJ_VECTOR});
  c_lib_t *c_lib = AS_C_LIB(argv[0])->lib;
  char *name = AS_CSTRING(argv[1]);

  c_type_wrapper_t *ret_type =
      (c_type_wrapper_t *)malloc(sizeof(c_type_wrapper_t));
  ret_type->points_to = NULL;
  if (!value_to_c_type_wrapper(argv[2], ret_type)) {
    runtime_error(-1, "Invalid return type");
    return NIL_VAL;
  }

  obj_vector_t *args = AS_VECTOR(argv[3]);
  c_type_wrapper_t **arg_types =
      (c_type_wrapper_t **)malloc(sizeof(c_type_wrapper_t *) * args->count);
  if (!arg_types) {
    runtime_error(-1, "Failed to allocate c arg types");
    return NIL_VAL;
  }

  for (int i = 0; i < args->count; i++) {
    c_type_wrapper_t *arg =
        (c_type_wrapper_t *)malloc(sizeof(c_type_wrapper_t));
    arg->points_to = NULL;
    if (!value_to_c_type_wrapper(args->values[i], arg)) {
      runtime_error(-1, "Invalid return type");
      return NIL_VAL;
    }

    arg_types[i] = arg;
  }

  c_func_t *c_func =
      cfunc_create(c_lib, name, ret_type, arg_types, args->count);
  if (!c_func) {
    runtime_error(-1, "Failed to create c func");
    free(arg_types);
    return NIL_VAL;
  }

  obj_c_func_t *func = new_c_func(c_func);
  if (!func) {
    runtime_error(-1, "Failed to create c func");
    cfunc_free(c_func);
    return NIL_VAL;
  }

  return OBJ_VAL(func);
}

xyl_builtin(ref) {
  xyl_builtin_signature(ref, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  obj_ref_t *ref = new_ref(argv[0]);
  return OBJ_VAL(ref);
}

xyl_builtin(deref) {
  xyl_builtin_signature(ref, 1, ARGC_EXACT, {VAL_OBJ, OBJ_REF});
  return AS_REF(argv[0])->val;
}

xyl_builtin(setref) {
  xyl_builtin_signature(setref, 2, ARGC_EXACT, {VAL_OBJ, OBJ_REF},
                        {VAL_ANY, OBJ_ANY});
  AS_REF(argv[0])->val = argv[1];
  return NIL_VAL;
}
