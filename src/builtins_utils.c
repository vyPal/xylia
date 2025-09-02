#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "compiler.h"
#include "memory.h"
#include "vm.h"

xyl_builtin(typeof) {
  xyl_builtin_signature(typeof, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});

  value_t value = argv[0];
  switch (value.type) {
  case VAL_BOOL:
    return OBJ_VAL(vm.vm_strings[VM_STR_BOOL]);
  case VAL_NIL:
    return OBJ_VAL(vm.vm_strings[VM_STR_NIL]);
  case VAL_NUMBER:
    return OBJ_VAL(vm.vm_strings[VM_STR_NUMBER]);
  case VAL_FLOAT:
    if (isnan(AS_FLOAT(value)))
      return OBJ_VAL(vm.vm_strings[VM_STR_NAN]);
    return OBJ_VAL(vm.vm_strings[VM_STR_FLOAT]);
  case VAL_OBJ:
    switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
      return OBJ_VAL(vm.vm_strings[VM_STR_STRING]);
    case OBJ_VECTOR:
      return OBJ_VAL(vm.vm_strings[VM_STR_VECTOR]);
    case OBJ_LIST:
      return OBJ_VAL(vm.vm_strings[VM_STR_LIST]);
    case OBJ_FILE:
      return OBJ_VAL(vm.vm_strings[VM_STR_FILE]);
    case OBJ_CLASS:
      return OBJ_VAL(vm.vm_strings[VM_STR_CLASS]);
    case OBJ_FUNCTION:
    case OBJ_BOUND_METHOD:
    case OBJ_CLOSURE:
      return OBJ_VAL(vm.vm_strings[VM_STR_FUNCTION]);
    case OBJ_INSTANCE:
      return OBJ_VAL(vm.vm_strings[VM_STR_INSTANCE]);
    case OBJ_BUILTIN:
      return OBJ_VAL(vm.vm_strings[VM_STR_BUILTIN]);
    case OBJ_UPVALUE:
      return OBJ_VAL(vm.vm_strings[VM_STR_UPVALUE]);
    case OBJ_MODULE:
      return OBJ_VAL(vm.vm_strings[VM_STR_MODULE]);
    case OBJ_RANGE:
      return OBJ_VAL(vm.vm_strings[VM_STR_RANGE]);
    case OBJ_ANY: // Unreachable
      break;
    }
  case VAL_ANY: // Unreachable
    break;
  }

  return NIL_VAL;
}

xyl_builtin(isinstance) {
  xyl_builtin_signature(isinstance, 2, ARGC_EXACT, {VAL_OBJ, OBJ_INSTANCE},
                        {VAL_OBJ, OBJ_CLASS});

  obj_instance_t *instance = AS_INSTANCE(argv[0]);
  obj_class_t *clas = AS_CLASS(argv[1]);

  return BOOL_VAL(instance->clas == clas);
}

xyl_builtin(exit) {
  xyl_builtin_signature(exit, 1, ARGC_LESS_OR_EXACT, {VAL_NUMBER, OBJ_ANY});

  if (argc == 1)
    set_signal(SIG_HALT, AS_NUMBER(argv[0]));
  else
    set_signal(SIG_HALT, -1);

  return NIL_VAL;
}

xyl_builtin(argv) {
  xyl_builtin_signature(argv, 0, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  return OBJ_VAL(vm.args);
}

xyl_builtin(import) {
  xyl_builtin_signature(argv, 1, ARGC_EXACT, {VAL_OBJ, OBJ_STRING});

  obj_string_t *path = AS_STRING(argv[0]);
  value_t value;
  if (table_get(&vm.module_lookup, path, &value))
    return value;

  if (path->length < EXT_LEN ||
      memcmp(path->chars + path->length - EXT_LEN, EXT, EXT_LEN) != 0) {
    const char *xyl_home = getenv("XYL_HOME");
    if (!xyl_home) {
      runtime_error("Could not find $XYL_HOME env variable");
      return NIL_VAL;
    }

    size_t xyl_home_len = strlen(xyl_home);
    char *lib_path =
        (char *)malloc(xyl_home_len + LIB_LEN + path->length + EXT_LEN + 1);
    memcpy(lib_path, xyl_home, xyl_home_len);
    memcpy(lib_path + xyl_home_len, LIB, LIB_LEN);
    memcpy(lib_path + xyl_home_len + LIB_LEN, path->chars, path->length);
    memcpy(lib_path + xyl_home_len + LIB_LEN + path->length, EXT, EXT_LEN);
    lib_path[xyl_home_len + LIB_LEN + path->length + EXT_LEN] = '\0';

    char *source = read_file(lib_path);
    free(lib_path);
    if (!source)
      return NIL_VAL;

    obj_module_t *module = compile(source, path);
    free(source);
    if (module == NULL) {
      runtime_error("Failed to compile module '%s'", path->chars);
      return NIL_VAL;
    }

    push_frame(module->init, 0);
    vm.frames[vm.frame_count - 1].is_module = true;
    vm.update_frame = true;

    push(OBJ_VAL(module));
    table_set(&vm.module_lookup, path, OBJ_VAL(module));
    pop();

    return OBJ_VAL(module);
  }

  char *source = read_file(path->chars);
  if (!source)
    return NIL_VAL;

  obj_module_t *module = compile(source, path);
  free(source);
  if (module == NULL) {
    runtime_error("Failed to compile module '%s'", path->chars);
    return NIL_VAL;
  }

  push_frame(module->init, 0);
  vm.frames[vm.frame_count - 1].is_module = true;
  vm.update_frame = true;

  push(OBJ_VAL(module));
  table_set(&vm.module_lookup, path, OBJ_VAL(module));
  pop();

  return OBJ_VAL(module);
}
