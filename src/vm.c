#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <threads.h>

#include "builtins.h"
#include "chunk.h"
#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#ifdef DECOMPILE
#include "debug.h"
#endif

vm_t vm;

void set_signal(vm_singal_t sig, int exit_code) {
  vm.signal = sig;
  vm.exit_code = exit_code;

  if (exit_code == -1) {
    switch (sig) {
    case SIG_STACK_OVERFLOW:
    case SIG_STACK_UNDERFLOW:
    case SIG_ASSERT_FAIL:
    case SIG_RUNTIME_ERROR:
    case SIG_TEST_ASSERT_FAIL:
      vm.exit_code = 1;
      break;
    case SIG_NONE:
    case SIG_HALT:
      vm.exit_code = 0;
      break;
    }
  }
}

void runtime_error(const char *fmt, ...) {
  fprintf(stderr, "[RUNTIME ERROR] ");
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fputs("\n", stderr);

  for (int i = vm.frame_count - 1; i >= 0; i--) {
    call_frame_t *frame = &vm.frames[i];
    obj_function_t *function = frame->closure->function;
    if (function->name == NULL)
      fprintf(stderr, "in script\n");
    else
      fprintf(stderr, "in %s()\n", function->name->chars);
  }

  set_signal(SIG_RUNTIME_ERROR, -1);
}

static void define_method(obj_string_t *name);

static void define_builtin(const char *name, builtin_fn_t function) {
  push(OBJ_VAL(copy_string(name, strlen(name), true)));
  push(OBJ_VAL(new_builtin(function)));
  table_set(&vm.builtins, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

static void init_stack(size_t initial_size) {
  vm.stack_capacity = initial_size > 0 ? initial_size : 64;

  vm.stack = ALLOCATE(value_t, vm.stack_capacity);
  vm.stack_top = vm.stack;
}

static void free_stack(void) {
  FREE_ARRAY(value_t, vm.stack, vm.stack_capacity);
  vm.stack = NULL;
  vm.stack_top = NULL;
  vm.stack_capacity = 0;
}

static void init_frames(size_t initial_size) {
  vm.frame_capacity = initial_size > 0 ? initial_size : 64;
  vm.frame_count = 0;

  vm.frames = ALLOCATE(call_frame_t, vm.frame_capacity);
}

static void free_frames(void) {
  FREE_ARRAY(call_frame_t, vm.frames, vm.frame_capacity);
  vm.frames = NULL;
  vm.frame_capacity = 0;
  vm.frame_count = 0;
}

static void init_vm_string(void) {
  for (int i = 0; i < VM_STR_MAX; i++)
    vm.vm_strings[i] = NULL;

  vm.vm_strings[VM_STR_INIT] = copy_string("init", 4, true);
  vm.vm_strings[VM_STR_BOOL] = copy_string("bool", 4, true);
  vm.vm_strings[VM_STR_NIL] = copy_string("nil", 3, true);
  vm.vm_strings[VM_STR_NUMBER] = copy_string("number", 6, true);
  vm.vm_strings[VM_STR_FLOAT] = copy_string("float", 5, true);
  vm.vm_strings[VM_STR_FUNCTION] = copy_string("function", 8, true);
  vm.vm_strings[VM_STR_CLASS] = copy_string("class", 5, true);
  vm.vm_strings[VM_STR_CLOSURE] = copy_string("closure", 7, true);
  vm.vm_strings[VM_STR_INSTANCE] = copy_string("instance", 8, true);
  vm.vm_strings[VM_STR_BUILTIN] = copy_string("builtin", 7, true);
  vm.vm_strings[VM_STR_STRING] = copy_string("string", 6, true);
  vm.vm_strings[VM_STR_UPVALUE] = copy_string("upvalue", 7, true);
  vm.vm_strings[VM_STR_VECTOR] = copy_string("vector", 6, true);
  vm.vm_strings[VM_STR_LIST] = copy_string("list", 4, true);
  vm.vm_strings[VM_STR_FILE] = copy_string("file", 4, true);
  vm.vm_strings[VM_STR_NAN] = copy_string("nan", 3, true);
  vm.vm_strings[VM_STR_MODULE] = copy_string("module", 6, true);
  vm.vm_strings[VM_STR_RANGE] = copy_string("range", 5, true);
  vm.vm_strings[VM_STR_OVERLOAD_EQ] = copy_string("__eq__", 6, true);
  vm.vm_strings[VM_STR_OVERLOAD_GT] = copy_string("__gt__", 6, true);
  vm.vm_strings[VM_STR_OVERLOAD_GE] = copy_string("__ge__", 6, true);
  vm.vm_strings[VM_STR_OVERLOAD_LT] = copy_string("__lt__", 6, true);
  vm.vm_strings[VM_STR_OVERLOAD_LE] = copy_string("__le__", 6, true);
  vm.vm_strings[VM_STR_OVERLOAD_ADD] = copy_string("__add__", 7, true);
  vm.vm_strings[VM_STR_OVERLOAD_SUB] = copy_string("__sub__", 7, true);
  vm.vm_strings[VM_STR_OVERLOAD_MUL] = copy_string("__mul__", 7, true);
  vm.vm_strings[VM_STR_OVERLOAD_DIV] = copy_string("__div__", 7, true);
  vm.vm_strings[VM_STR_OVERLOAD_NEG] = copy_string("__neg__", 7, true);
  vm.vm_strings[VM_STR_OVERLOAD_MOD] = copy_string("__mod__", 7, true);
  vm.vm_strings[VM_STR_OVERLOAD_XOR] = copy_string("__xor__", 7, true);
  vm.vm_strings[VM_STR_OVERLOAD_BIT_OR] = copy_string("__bin_or__", 10, true);
  vm.vm_strings[VM_STR_OVERLOAD_BIT_AND] = copy_string("__bin_and__", 11, true);
  vm.vm_strings[VM_STR_OVERLOAD_BIT_NOT] = copy_string("__bin_not__", 11, true);
  vm.vm_strings[VM_STR_OVERLOAD_LOG_NOT] = copy_string("__log_not__", 11, true);
  vm.vm_strings[VM_STR_OVERLOAD_SET_INDEX] =
      copy_string("__set_index__", 13, true);
  vm.vm_strings[VM_STR_OVERLOAD_GET_INDEX] =
      copy_string("__get_index__", 13, true);
  vm.vm_strings[VM_STR_OVERLOAD_SET_SLICE] =
      copy_string("__set_slice__", 13, true);
  vm.vm_strings[VM_STR_OVERLOAD_GET_SLICE] =
      copy_string("__get_slice__", 13, true);
}

void init_vm(void) {
  init_stack(0);
  init_frames(0);

  vm.bytes_allocated = 0;
  vm.next_gc = 1024 * 1024;
  vm.objects = NULL;

  vm.gray_capacity = 0;
  vm.gray_count = 0;
  vm.gray_stack = NULL;

  vm.args = NULL;

  // init_table(&vm.globals);
  init_value_array(&vm.modules);
  init_table(&vm.builtins);
  init_table(&vm.strings);

  init_vm_string();

  define_builtin("print", builtin_print);
  define_builtin("println", builtin_println);
  define_builtin("printf", builtin_printf);
  define_builtin("input", builtin_input);

  define_builtin("open", builtin_open);
  define_builtin("close", builtin_close);
  define_builtin("read", builtin_read);
  define_builtin("write", builtin_write);

  define_builtin("len", builtin_len);
  define_builtin("append", builtin_append);
  define_builtin("pop", builtin_pop);
  define_builtin("insert", builtin_insert);
  define_builtin("remove", builtin_remove);
  define_builtin("slice", builtin_slice);

  define_builtin("typeof", builtin_typeof);
  define_builtin("isinstance", builtin_isinstance);
  define_builtin("exit", builtin_exit);
  define_builtin("argv", builtin_argv);

  define_builtin("import", builtin_import);

  define_builtin("string", builtin_string);
  define_builtin("number", builtin_number);
  define_builtin("float", builtin_float);
  define_builtin("bool", builtin_bool);
  define_builtin("vector", builtin_vector);
  define_builtin("list", builtin_list);

  define_builtin("case_failed", builtin_case_failed);

  define_builtin("assert_true", builtin_assert_true);
  define_builtin("assert_false", builtin_assert_false);
  define_builtin("assert_eq", builtin_assert_eq);
  define_builtin("assert_neq", builtin_assert_neq);

  set_signal(SIG_NONE, -1);
  vm.update_frame = false;
}

void free_vm(void) {
  free_stack();
  free_frames();

  // free_table(&vm.globals);
  free_value_array(&vm.modules);
  free_table(&vm.builtins);
  free_table(&vm.strings);

  for (int i = 0; i < VM_STR_MAX; i++)
    vm.vm_strings[i] = NULL;

  free_objects();
}

void set_args(int argc, char **argv) {
  vm.args = new_list(argc);
  for (int i = 0; i < argc; i++)
    vm.args->values[i] = OBJ_VAL(copy_string(argv[i], strlen(argv[i]), true));
}

void push(value_t value) {
  size_t count = vm.stack_top - vm.stack;
  if (count >= vm.stack_capacity) {
    int old_capacity = vm.stack_capacity;
    vm.stack_capacity = GROW_CAPACITY(old_capacity);
    vm.stack = GROW_ARRAY(value_t, vm.stack, old_capacity, vm.stack_capacity);
  }

  vm.stack_top = vm.stack + count;
  *vm.stack_top = value;
  vm.stack_top++;
}

value_t pop(void) {
  if (vm.stack_top - vm.stack <= 0) {
    set_signal(SIG_STACK_UNDERFLOW, -1);
    return NIL_VAL;
  }

  vm.stack_top--;
  return *vm.stack_top;
}

void push_frame(obj_closure_t *closure, int argc) {
  if (vm.frame_count >= vm.frame_capacity) {
    int old_capacity = vm.frame_capacity;
    vm.frame_capacity = GROW_CAPACITY(old_capacity);
    vm.frames =
        GROW_ARRAY(call_frame_t, vm.frames, old_capacity, vm.frame_capacity);
  }

  call_frame_t *frame = &vm.frames[vm.frame_count++];
  frame->closure = closure;
  frame->globals = closure->function->globals;
  frame->ip = closure->function->chunk.code;
  frame->slots = vm.stack_top - argc - 1;
  frame->is_module = false;
}

static value_t peek(int distance) {
  if (vm.stack_top - vm.stack - 1 - distance < 0) {
    set_signal(SIG_STACK_UNDERFLOW, -1);
    return NIL_VAL;
  } else if (distance < 0) {
    set_signal(SIG_STACK_OVERFLOW, -1);
    return NIL_VAL;
  }

  return vm.stack_top[-1 - distance];
}

static bool call(obj_closure_t *closure, int argc) {
  int true_argc = 0;
  for (int i = 0; i < argc; i++) {
    if (IS_LIST(peek(i)) && AS_LIST(peek(i))->spread)
      true_argc += AS_LIST(peek(i))->count;
    else if (IS_VECTOR(peek(i)) && AS_VECTOR(peek(i))->spread)
      true_argc += AS_VECTOR(peek(i))->count;
    else
      true_argc++;
  }

  if (closure->function->has_varargs) {
    if (true_argc < closure->function->arity - 1) {
      runtime_error("Expected at least %d arguments but got %d",
                    closure->function->arity, true_argc);
      return false;
    }
  } else if (true_argc != closure->function->arity) {
    runtime_error("Expected %d arguments but got %d", closure->function->arity,
                  true_argc);
    return false;
  }

  value_t stack[argc] = {};
  for (int i = 0; i < argc; i++)
    stack[i] = pop();

  for (int i = argc - 1; i >= 0; i--) {
    value_t value = stack[i];
    if (IS_LIST(value) && AS_LIST(value)->spread) {
      for (int j = 0; j < AS_LIST(value)->count; j++)
        push(AS_LIST(value)->values[j]);
    } else if (IS_VECTOR(value) && AS_VECTOR(value)->spread) {
      for (int j = 0; j < AS_VECTOR(value)->count; j++)
        push(AS_VECTOR(value)->values[j]);
    } else
      push(value);
  }

  if (closure->function->has_varargs) {
    int vararg_count = true_argc - (closure->function->arity - 1);
    obj_list_t *list = new_list(vararg_count);
    for (int i = vararg_count - 1; i >= 0; i--)
      list->values[i] = pop();
    push(OBJ_VAL(list));

    push_frame(closure, closure->function->arity);
  } else
    push_frame(closure, true_argc);

  return true;
}

static bool call_value(value_t callee, int argc) {
  if (IS_OBJ(callee)) {
    switch (OBJ_TYPE(callee)) {
    case OBJ_BOUND_METHOD: {
      obj_bound_method_t *bound = AS_BOUND_METHOD(callee);
      vm.stack_top[-argc - 1] = bound->receiver;
      return call(bound->method, argc);
    }
    case OBJ_CLASS: {
      obj_class_t *clas = AS_CLASS(callee);
      vm.stack_top[-argc - 1] = OBJ_VAL(new_instance(clas));
      value_t initializer;
      if (table_get(&clas->methods, vm.vm_strings[VM_STR_INIT], &initializer))
        return call(AS_CLOSURE(initializer), argc);
      else if (argc != 0) {
        runtime_error("Expected 0 arguments but got %d", argc);
        return false;
      }
      return true;
    }
    case OBJ_CLOSURE:
      return call(AS_CLOSURE(callee), argc);
    case OBJ_BUILTIN: {
      builtin_fn_t builtin = AS_BUILTIN(callee);
      value_t result = builtin(argc, vm.stack_top - argc);
      vm.stack_top -= argc + 1;
      push(result);
      return true;
    }
    default:
      break;
    }
  }

  runtime_error("Can only call functions and classes");
  return false;
}

static bool invoke_from_class(obj_class_t *clas, obj_string_t *name, int argc) {
  value_t method;
  if (!table_get(&clas->methods, name, &method)) {
    runtime_error("Undefined property '%s'", name->chars);
    return false;
  }
  return call(AS_CLOSURE(method), argc);
}

static bool invoke(obj_string_t *name, int argc) {
  value_t receiver = peek(argc);

  if (!IS_INSTANCE(receiver) && !IS_MODULE(receiver)) {
    runtime_error("Only instances and modules have methods");
    return false;
  }

  if (IS_INSTANCE(receiver)) {
    obj_instance_t *instance = AS_INSTANCE(receiver);

    value_t value;
    if (table_get(&instance->fields, name, &value)) {
      vm.stack_top[-argc - 1] = value;
      return call_value(value, argc);
    }

    return invoke_from_class(instance->clas, name, argc);
  } else if (IS_MODULE(receiver)) {
    obj_module_t *moduele = AS_MODULE(receiver);

    value_t value;
    if (table_get(&moduele->globals, name, &value)) {
      vm.stack_top[-argc - 1] = value;
      return call_value(value, argc);
    }
  }

  runtime_error("Only instances and modules have methods");
  return false;
}

static bool invoke_overload(vm_strings_t overload, int argc) {
  if (!IS_INSTANCE(peek(argc)))
    return false;

  obj_string_t *name = vm.vm_strings[overload];
  if (!invoke(name, argc))
    return false;
  return true;
}

static bool bind_method(obj_class_t *clas, obj_string_t *name) {
  value_t method;
  if (!table_get(&clas->methods, name, &method)) {
    runtime_error("Undefined property '%s'", name->chars);
    return false;
  }

  obj_bound_method_t *bound = new_bound_method(peek(0), AS_CLOSURE(method));
  pop();
  push(OBJ_VAL(bound));
  return true;
}

static obj_upvalue_t *capture_upvalue(value_t *local) {
  obj_upvalue_t *prev_upvalue = NULL;
  obj_upvalue_t *upvalue = vm.open_upvalues;
  while (upvalue != NULL && upvalue->location > local) {
    prev_upvalue = upvalue;
    upvalue = upvalue->next;
  }

  if (upvalue != NULL && upvalue->location == local)
    return upvalue;

  obj_upvalue_t *created_upvalue = new_upvalue(local);
  created_upvalue->next = upvalue;

  if (prev_upvalue == NULL)
    vm.open_upvalues = created_upvalue;
  else
    prev_upvalue->next = created_upvalue;

  return created_upvalue;
}

static void close_upvalues(value_t *last) {
  while (vm.open_upvalues != NULL && vm.open_upvalues->location >= last) {
    obj_upvalue_t *upvalue = vm.open_upvalues;
    upvalue->closed = *upvalue->location;
    upvalue->location = &upvalue->closed;
    vm.open_upvalues = upvalue->next;
  }
}

static void define_method(obj_string_t *name) {
  value_t method = peek(0);
  obj_class_t *clas = AS_CLASS(peek(1));
  table_set(&clas->methods, name, method);
  pop();
}

static bool is_falsey(value_t value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate(void) {
  obj_string_t *b = AS_STRING(peek(0));
  obj_string_t *a = AS_STRING(peek(1));

  int length = a->length + b->length;
  char *chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  obj_string_t *result = take_string(chars, length);
  pop();
  pop();
  push(OBJ_VAL(result));
}

static value_t get_index(value_t object, int index) {
  if (IS_STRING(object)) {
    obj_string_t *string = AS_STRING(object);
    if (index < 0 || index >= string->length) {
      runtime_error("String index '%d' out of bounds", index);
      return NIL_VAL;
    }
    char c[2] = {string->chars[index], '\0'};
    return OBJ_VAL(copy_string(c, 1, true));
  } else if (IS_VECTOR(object)) {
    obj_vector_t *vector = AS_VECTOR(object);
    if (index < 0 || index >= vector->count) {
      runtime_error("Vector index '%d' out of bounds", index);
      return NIL_VAL;
    }
    return vector->values[index];
  } else if (IS_LIST(object)) {
    obj_list_t *list = AS_LIST(object);
    if (index < 0 || index >= list->count) {
      runtime_error("List index '%d' out of bounds", index);
      return NIL_VAL;
    }
    return list->values[index];
  }

  runtime_error("Invalid index operation");
  return NIL_VAL;
}

static void set_index(value_t object, int index, value_t value) {
  if (IS_VECTOR(object)) {
    obj_vector_t *vector = AS_VECTOR(object);
    if (index < 0 || index >= vector->count) {
      runtime_error("Vector index '%d' out of bounds", index);
      return;
    }
    vector->values[index] = value;
  } else {
    runtime_error("Invalid index operation");
  }
}

static result_t run(void) {
  call_frame_t *frame = &vm.frames[vm.frame_count - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() (READ_BYTE() | (READ_BYTE() << 8))
#define READ_LONG() (READ_BYTE() | (READ_BYTE() << 8) | (READ_BYTE() << 16))
#define READ_CONSTANT()                                                        \
  (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_CONSTANT_LONG()                                                   \
  (frame->closure->function->chunk.constants.values[READ_LONG()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_STRING_LONG() AS_STRING(READ_CONSTANT_LONG())

#define IS_NUM_OR_FLT(value) (IS_NUMBER(value) || IS_FLOAT(value))
#define UPDATE_FRAME() frame = &vm.frames[vm.frame_count - 1]

  while (true) {
#ifdef DECOMPILE
    printf("     ");
    for (value_t *slot = vm.stack; slot < vm.stack_top; slot++) {
      printf("[ ");
      print_value(*slot, true);
      printf(" ]");
    }
    printf("\n");
    disassemble_instruction(&frame->closure->function->chunk,
                            frame->ip - frame->closure->function->chunk.code);
#endif
    // table_t *globals = frame->globals == NULL ? &vm.globals : frame->globals;
    table_t *globals = frame->globals;
    if (globals == NULL) {
      runtime_error("No globals table found");
      return RESULT_RUNTIME_ERROR;
    }

    switch (READ_BYTE()) {
    case OP_CONSTANT:
      push(READ_CONSTANT());
      break;
    case OP_CONSTANT_LONG:
      push(READ_CONSTANT_LONG());
      break;
    case OP_DEFINE_GLOBAL: {
      obj_string_t *name = READ_STRING();
      table_set(globals, name, peek(0));
      pop();
    } break;
    case OP_DEFINE_GLOBAL_LONG: {
      obj_string_t *name = READ_STRING_LONG();
      table_set(globals, name, peek(0));
      pop();
    } break;
    case OP_GET_GLOBAL: {
      obj_string_t *name = READ_STRING();
      value_t value;
      if (!table_get(globals, name, &value)) {
        if (!table_get(&vm.builtins, name, &value)) {
          runtime_error("Undefined variable '%s'", name->chars);
          return RESULT_RUNTIME_ERROR;
        }
      }
      push(value);
    } break;
    case OP_GET_GLOBAL_LONG: {
      obj_string_t *name = READ_STRING_LONG();
      value_t value;
      if (!table_get(globals, name, &value)) {
        if (!table_get(&vm.builtins, name, &value)) {
          runtime_error("Undefined variable '%s'", name->chars);
          return RESULT_RUNTIME_ERROR;
        }
      }
      push(value);
    } break;
    case OP_SET_GLOBAL: {
      obj_string_t *name = READ_STRING();
      if (table_set(globals, name, peek(0))) {
        table_delete(globals, name);
        runtime_error("Undefined variable '%s'", name->chars);
        return RESULT_RUNTIME_ERROR;
      }
    } break;
    case OP_SET_GLOBAL_LONG: {
      obj_string_t *name = READ_STRING_LONG();
      if (table_set(globals, name, peek(0))) {
        table_delete(globals, name);
        runtime_error("Undefined variable '%s'", name->chars);
        return RESULT_RUNTIME_ERROR;
      }
    } break;
    case OP_GET_LOCAL: {
      unsigned int slot = READ_BYTE();
      push(frame->slots[slot]);
    } break;
    case OP_GET_LOCAL_LONG: {
      unsigned int slot = READ_LONG();
      push(frame->slots[slot]);
    } break;
    case OP_SET_LOCAL: {
      unsigned int slot = READ_BYTE();
      frame->slots[slot] = peek(0);
    } break;
    case OP_SET_LOCAL_LONG: {
      unsigned int slot = READ_LONG();
      frame->slots[slot] = peek(0);
    } break;
    case OP_GET_UPVALUE: {
      unsigned int slot = READ_BYTE();
      push(*frame->closure->upvalues[slot]->location);
    } break;
    case OP_GET_UPVALUE_LONG: {
      unsigned int slot = READ_LONG();
      push(*frame->closure->upvalues[slot]->location);
    } break;
    case OP_SET_UPVALUE: {
      unsigned int slot = READ_BYTE();
      *frame->closure->upvalues[slot]->location = peek(0);
    } break;
    case OP_SET_UPVALUE_LONG: {
      unsigned int slot = READ_LONG();
      *frame->closure->upvalues[slot]->location = peek(0);
    } break;
    case OP_GET_SUPER: {
      obj_string_t *name = READ_STRING();
      obj_class_t *super_class = AS_CLASS(pop());

      if (!bind_method(super_class, name))
        return RESULT_RUNTIME_ERROR;
    } break;
    case OP_GET_SUPER_LONG: {
      obj_string_t *name = READ_STRING_LONG();
      obj_class_t *super_class = AS_CLASS(pop());

      if (!bind_method(super_class, name))
        return RESULT_RUNTIME_ERROR;
    } break;
    case OP_GET_PROPERTY: {
      if (!IS_INSTANCE(peek(0)) && !IS_MODULE(peek(0))) {
        runtime_error("Only instances and modules have properties");
        return RESULT_RUNTIME_ERROR;
      }

      if (IS_INSTANCE(peek(0))) {
        obj_instance_t *instance = AS_INSTANCE(peek(0));
        obj_string_t *name = READ_STRING();

        value_t value;
        if (table_get(&instance->fields, name, &value)) {
          pop();
          push(value);
          break;
        }

        if (!bind_method(instance->clas, name))
          return RESULT_RUNTIME_ERROR;
      } else if (IS_MODULE(peek(0))) {
        obj_module_t *module = AS_MODULE(peek(0));
        obj_string_t *name = READ_STRING();

        value_t value;
        if (table_get(&module->globals, name, &value)) {
          pop();
          push(value);
          break;
        }

        runtime_error("Object does not have property '%s'", name->chars);
        return RESULT_RUNTIME_ERROR;
      }
    } break;
    case OP_GET_PROPERTY_LONG: {
      if (!IS_INSTANCE(peek(0)) && !IS_MODULE(peek(0))) {
        runtime_error("Only instances and modules have properties");
        return RESULT_RUNTIME_ERROR;
      }

      if (IS_INSTANCE(peek(0))) {
        obj_instance_t *instance = AS_INSTANCE(peek(0));
        obj_string_t *name = READ_STRING_LONG();

        value_t value;
        if (table_get(&instance->fields, name, &value)) {
          pop();
          push(value);
          break;
        }

        if (!bind_method(instance->clas, name))
          return RESULT_RUNTIME_ERROR;
      } else if (IS_MODULE(peek(0))) {
        obj_module_t *module = AS_MODULE(peek(0));
        obj_string_t *name = READ_STRING_LONG();

        value_t value;
        if (table_get(&module->globals, name, &value)) {
          pop();
          push(value);
          break;
        }

        runtime_error("Object does not have property '%s'", name->chars);
        return RESULT_RUNTIME_ERROR;
      }
    } break;
    case OP_SET_PROPERTY: {
      if (!IS_INSTANCE(peek(1))) {
        runtime_error("Only instances have fields");
        return RESULT_RUNTIME_ERROR;
      }

      obj_instance_t *instance = AS_INSTANCE(peek(1));
      table_set(&instance->fields, READ_STRING(), peek(0));
      value_t value = pop();
      pop();
      push(value);
    } break;
    case OP_SET_PROPERTY_LONG: {
      if (!IS_INSTANCE(peek(1))) {
        runtime_error("Only instances have fields");
        return RESULT_RUNTIME_ERROR;
      }

      obj_instance_t *instance = AS_INSTANCE(peek(1));
      table_set(&instance->fields, READ_STRING_LONG(), peek(0));
      value_t value = pop();
      pop();
      push(value);
    } break;
    case OP_GET_INDEX: {
      value_t index = peek(0);
      value_t object = peek(1);

      if (IS_INSTANCE(object)) {
        if (IS_RANGE(index)) {
          obj_range_t *range = AS_RANGE(pop());
          push(range->from);
          push(range->to);
          if (invoke_overload(VM_STR_OVERLOAD_GET_SLICE, 2)) {
            UPDATE_FRAME();
            break;
          }
        } else if (invoke_overload(VM_STR_OVERLOAD_GET_INDEX, 1)) {
          UPDATE_FRAME();
          break;
        }
      }

      if (!IS_NUMBER(index)) {
        runtime_error("Index must be a number or object with 'operator []'");
        return RESULT_RUNTIME_ERROR;
      }

      value_t result = get_index(object, AS_NUMBER(index));

      pop();
      pop();
      push(result);
    } break;
    case OP_SET_INDEX: {
      value_t value = peek(0);
      value_t index = peek(1);
      value_t object = peek(2);

      if (IS_INSTANCE(object)) {
        if (IS_RANGE(index)) {
          pop();
          obj_range_t *range = AS_RANGE(pop());
          push(range->from);
          push(range->to);
          push(value);
          if (invoke_overload(VM_STR_OVERLOAD_SET_SLICE, 3)) {
            UPDATE_FRAME();
            break;
          }
        } else if (invoke_overload(VM_STR_OVERLOAD_SET_INDEX, 2)) {
          UPDATE_FRAME();
          break;
        }
      }

      if (!IS_NUMBER(index)) {
        runtime_error("Index must be a number or object with 'operator []='");
        return RESULT_RUNTIME_ERROR;
      }

      set_index(object, AS_NUMBER(index), value);

      pop();
      pop();
      pop();
      push(value);
    } break;
    case OP_INVOKE: {
      obj_string_t *method = READ_STRING();
      unsigned int argc = READ_BYTE();
      if (!invoke(method, argc))
        return RESULT_RUNTIME_ERROR;
      UPDATE_FRAME();
    } break;
    case OP_INVOKE_LONG: {
      obj_string_t *method = READ_STRING_LONG();
      unsigned int argc = READ_BYTE();
      if (!invoke(method, argc))
        return RESULT_RUNTIME_ERROR;
      UPDATE_FRAME();
    } break;
    case OP_SUPER_INVOKE: {
      obj_string_t *method = READ_STRING();
      unsigned int argc = READ_BYTE();
      obj_class_t *super_class = AS_CLASS(pop());
      if (!invoke_from_class(super_class, method, argc))
        return RESULT_RUNTIME_ERROR;
      UPDATE_FRAME();
    } break;
    case OP_SUPER_INVOKE_LONG: {
      obj_string_t *method = READ_STRING_LONG();
      unsigned int argc = READ_BYTE();
      obj_class_t *super_class = AS_CLASS(pop());
      if (!invoke_from_class(super_class, method, argc))
        return RESULT_RUNTIME_ERROR;
      UPDATE_FRAME();
    } break;
    case OP_VECTOR: {
      unsigned int size = READ_BYTE();
      obj_vector_t *vector = new_vector(size);
      vector->count = size;
      for (int i = size - 1; i >= 0; i--)
        vector->values[i] = pop();
      push(OBJ_VAL(vector));
    } break;
    case OP_VECTOR_LONG: {
      unsigned int size = READ_LONG();
      obj_vector_t *vector = new_vector(size);
      vector->count = size;
      for (int i = size - 1; i >= 0; i--)
        vector->values[i] = pop();
      push(OBJ_VAL(vector));
    } break;
    case OP_LIST: {
      unsigned int size = READ_BYTE();
      obj_list_t *list = new_list(size);
      for (int i = size - 1; i >= 0; i--)
        list->values[i] = pop();
      push(OBJ_VAL(list));
    } break;
    case OP_LIST_LONG: {
      unsigned int size = READ_LONG();
      obj_list_t *list = new_list(size);
      for (int i = size - 1; i >= 0; i--)
        list->values[i] = pop();
      push(OBJ_VAL(list));
    } break;
    case OP_CLASS:
      push(OBJ_VAL(new_class(READ_STRING())));
      break;
    case OP_CLASS_LONG:
      push(OBJ_VAL(new_class(READ_STRING_LONG())));
      break;
    case OP_CLOSURE: {
      obj_function_t *function = AS_FUNCTION(READ_CONSTANT());
      obj_closure_t *closure = new_closure(function);
      push(OBJ_VAL(closure));
      for (int i = 0; i < closure->upvalue_count; i++) {
        uint8_t is_local = READ_BYTE();
        uint8_t index = READ_BYTE();
        if (is_local)
          closure->upvalues[i] = capture_upvalue(frame->slots + index);
        else
          closure->upvalues[i] = frame->closure->upvalues[index];
      }
    } break;
    case OP_CLOSURE_LONG: {
      obj_function_t *function = AS_FUNCTION(READ_CONSTANT_LONG());
      obj_closure_t *closure = new_closure(function);
      push(OBJ_VAL(closure));
      for (int i = 0; i < closure->upvalue_count; i++) {
        uint8_t is_local = READ_BYTE();
        uint8_t index = READ_BYTE();
        if (is_local)
          closure->upvalues[i] = capture_upvalue(frame->slots + index);
        else
          closure->upvalues[i] = frame->closure->upvalues[index];
      }
    } break;
    case OP_METHOD:
      define_method(READ_STRING());
      break;
    case OP_METHOD_LONG:
      define_method(READ_STRING_LONG());
      break;
    case OP_TRUE:
      push(BOOL_VAL(true));
      break;
    case OP_FALSE:
      push(BOOL_VAL(false));
      break;
    case OP_NIL:
      push(NIL_VAL);
      break;
    case OP_POP:
      pop();
      break;
    case OP_SPREAD: {
      if (IS_LIST(peek(0)))
        AS_LIST(peek(0))->spread = true;
      else if (IS_VECTOR(peek(0)))
        AS_VECTOR(peek(0))->spread = true;
      else {
        runtime_error("Can spread only 'list' and 'vector'");
        return RESULT_RUNTIME_ERROR;
      }
    } break;
    case OP_RANGE: {
      obj_range_t *range = new_range(peek(1), peek(0));
      pop();
      pop();
      push(OBJ_VAL(range));
    } break;
    case OP_ADD: {
      if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
        concatenate();
      } else if (IS_NUM_OR_FLT(peek(0)) && IS_NUM_OR_FLT(peek(1))) {
        value_t b = pop();
        value_t a = pop();

        double b_flt = IS_FLOAT(b) ? AS_FLOAT(b) : (double)AS_NUMBER(b);
        double a_flt = IS_FLOAT(a) ? AS_FLOAT(a) : (double)AS_NUMBER(a);

        if (IS_NUMBER(a) && IS_NUMBER(b))
          push(NUMBER_VAL((int64_t)(a_flt + b_flt)));
        else
          push(FLOAT_VAL(a_flt + b_flt));
      } else {
        if (invoke_overload(VM_STR_OVERLOAD_ADD, 1))
          UPDATE_FRAME();
        else {
          runtime_error(
              "Operands must be numbers, floats, strings or objects with "
              "'operator +' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_SUB: {
      if (IS_NUM_OR_FLT(peek(0)) && IS_NUM_OR_FLT(peek(1))) {
        value_t b = pop();
        value_t a = pop();

        double b_flt = IS_FLOAT(b) ? AS_FLOAT(b) : (double)AS_NUMBER(b);
        double a_flt = IS_FLOAT(a) ? AS_FLOAT(a) : (double)AS_NUMBER(a);

        if (IS_NUMBER(a) && IS_NUMBER(b))
          push(NUMBER_VAL((int64_t)(a_flt - b_flt)));
        else
          push(FLOAT_VAL(a_flt - b_flt));
      } else {
        if (invoke_overload(VM_STR_OVERLOAD_SUB, 1))
          UPDATE_FRAME();
        else {
          runtime_error("Operands must be numbers, floats, or objects with "
                        "'operator -' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_MUL: {
      if (IS_NUM_OR_FLT(peek(0)) && IS_NUM_OR_FLT(peek(1))) {
        value_t b = pop();
        value_t a = pop();

        double b_flt = IS_FLOAT(b) ? AS_FLOAT(b) : (double)AS_NUMBER(b);
        double a_flt = IS_FLOAT(a) ? AS_FLOAT(a) : (double)AS_NUMBER(a);

        if (IS_NUMBER(a) && IS_NUMBER(b))
          push(NUMBER_VAL((int64_t)(a_flt * b_flt)));
        else
          push(FLOAT_VAL(a_flt * b_flt));
      } else {
        if (invoke_overload(VM_STR_OVERLOAD_MUL, 1))
          UPDATE_FRAME();
        else {
          runtime_error("Operands must be numbers, floats or objects with "
                        "'operator *' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_DIV: {
      if (IS_NUM_OR_FLT(peek(0)) && IS_NUM_OR_FLT(peek(1))) {
        value_t b = pop();
        value_t a = pop();

        double b_flt = IS_FLOAT(b) ? AS_FLOAT(b) : (double)AS_NUMBER(b);
        double a_flt = IS_FLOAT(a) ? AS_FLOAT(a) : (double)AS_NUMBER(a);

        push(FLOAT_VAL(a_flt / b_flt));
      } else {
        if (invoke_overload(VM_STR_OVERLOAD_DIV, 1))
          UPDATE_FRAME();
        else {
          runtime_error("Operands must be numbers, floats or objects with "
                        "'operator /' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_MOD: {
      if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
        value_t b = pop();
        value_t a = pop();
        push(NUMBER_VAL(AS_NUMBER(a) % AS_NUMBER(b)));
      } else if (IS_NUM_OR_FLT(peek(0)) && IS_NUM_OR_FLT(peek(1))) {
        value_t b = pop();
        value_t a = pop();

        double b_flt = IS_FLOAT(b) ? AS_FLOAT(b) : (double)AS_NUMBER(b);
        double a_flt = IS_FLOAT(a) ? AS_FLOAT(a) : (double)AS_NUMBER(a);

        push(FLOAT_VAL(fmod(a_flt, b_flt)));
      } else {
        if (invoke_overload(VM_STR_OVERLOAD_MOD, 1))
          UPDATE_FRAME();
        else {
          runtime_error("Operands must be numbers, floats or objects with "
                        "'operator %' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_BIT_AND: {
      if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
        int64_t b = AS_NUMBER(pop());
        int64_t a = AS_NUMBER(pop());
        push(NUMBER_VAL(a & b));
      } else {
        if (invoke_overload(VM_STR_OVERLOAD_BIT_AND, 1))
          UPDATE_FRAME();
        else {
          runtime_error("Operands must be numbers or objects with 'operator "
                        "&' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_BIT_OR: {
      if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
        int64_t b = AS_NUMBER(pop());
        int64_t a = AS_NUMBER(pop());
        push(NUMBER_VAL(a | b));
      } else {
        if (invoke_overload(VM_STR_OVERLOAD_BIT_OR, 1))
          UPDATE_FRAME();
        else {
          runtime_error("Operands must be numbers or objects with 'operator "
                        "|' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_XOR: {
      if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
        int64_t b = AS_NUMBER(pop());
        int64_t a = AS_NUMBER(pop());
        push(NUMBER_VAL(a ^ b));
      } else {
        if (invoke_overload(VM_STR_OVERLOAD_XOR, 1))
          UPDATE_FRAME();
        else {
          runtime_error("Operands must be numbers or objects with 'operator "
                        "^' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_EQ: {
      value_t b = peek(0);
      value_t a = peek(1);

      if (IS_INSTANCE(a)) {
        if (invoke_overload(VM_STR_OVERLOAD_EQ, 1)) {
          UPDATE_FRAME();
          break;
        }
      }

      pop();
      pop();
      push(BOOL_VAL(values_equal(a, b)));
    } break;
    case OP_GT: {
      if (IS_NUM_OR_FLT(peek(0)) && IS_NUM_OR_FLT(peek(1))) {
        value_t b = pop();
        value_t a = pop();

        double b_flt = IS_FLOAT(b) ? AS_FLOAT(b) : (double)AS_NUMBER(b);
        double a_flt = IS_FLOAT(a) ? AS_FLOAT(a) : (double)AS_NUMBER(a);

        push(BOOL_VAL(a_flt > b_flt));
      } else {
        if (invoke_overload(VM_STR_OVERLOAD_GT, 1))
          UPDATE_FRAME();
        else {
          runtime_error("Operands must be numbers, floats or objects with "
                        "'operator >' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_GE: {
      if (IS_NUM_OR_FLT(peek(0)) && IS_NUM_OR_FLT(peek(1))) {
        value_t b = pop();
        value_t a = pop();

        double b_flt = IS_FLOAT(b) ? AS_FLOAT(b) : (double)AS_NUMBER(b);
        double a_flt = IS_FLOAT(a) ? AS_FLOAT(a) : (double)AS_NUMBER(a);

        push(BOOL_VAL(a_flt >= b_flt));
      } else {
        if (invoke_overload(VM_STR_OVERLOAD_GE, 1))
          UPDATE_FRAME();
        else {
          runtime_error("Operands must be numbers, floats or objects with "
                        "'operator >=' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_LT: {
      if (IS_NUM_OR_FLT(peek(0)) && IS_NUM_OR_FLT(peek(1))) {
        value_t b = pop();
        value_t a = pop();

        double b_flt = IS_FLOAT(b) ? AS_FLOAT(b) : (double)AS_NUMBER(b);
        double a_flt = IS_FLOAT(a) ? AS_FLOAT(a) : (double)AS_NUMBER(a);

        push(BOOL_VAL(a_flt < b_flt));
      } else {
        if (invoke_overload(VM_STR_OVERLOAD_LT, 1))
          UPDATE_FRAME();
        else {
          runtime_error("Operands must be numbers, floats or objects with "
                        "'operator <' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_LE: {
      if (IS_NUM_OR_FLT(peek(0)) && IS_NUM_OR_FLT(peek(1))) {
        value_t b = pop();
        value_t a = pop();

        double b_flt = IS_FLOAT(b) ? AS_FLOAT(b) : (double)AS_NUMBER(b);
        double a_flt = IS_FLOAT(a) ? AS_FLOAT(a) : (double)AS_NUMBER(a);

        push(BOOL_VAL(a_flt <= b_flt));
      } else {
        if (invoke_overload(VM_STR_OVERLOAD_LE, 1))
          UPDATE_FRAME();
        else {
          runtime_error("Operands must be numbers, floats or objects with "
                        "'operator <=' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_NEG: {
      if (!IS_NUM_OR_FLT(peek(0))) {
        if (IS_INSTANCE(peek(0))) {
          if (invoke_overload(VM_STR_OVERLOAD_NEG, 0)) {
            UPDATE_FRAME();
            break;
          }
        }
        runtime_error("Operand must be number, float or object with 'operator "
                      "unary-' defined");
        return RESULT_RUNTIME_ERROR;
      }

      if (IS_NUMBER(peek(0)))
        push(NUMBER_VAL(-AS_NUMBER(pop())));
      else
        push(FLOAT_VAL(-AS_FLOAT(pop())));
    } break;
    case OP_LOG_NOT: {
      if (IS_NIL(peek(0)) || IS_BOOL(peek(0)))
        push(BOOL_VAL(is_falsey(pop())));
      else {
        if (invoke_overload(VM_STR_OVERLOAD_LOG_NOT, 0))
          UPDATE_FRAME();
        else {
          runtime_error(
              "Operand must be boolean or object with 'operator !' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_BIT_NOT: {
      if (IS_NUMBER(peek(0)))
        push(NUMBER_VAL(~AS_NUMBER(pop())));
      else {
        if (invoke_overload(VM_STR_OVERLOAD_BIT_NOT, 0))
          UPDATE_FRAME();
        else {
          runtime_error(
              "Operand must be number or object with 'operator ~' defined");
          return RESULT_RUNTIME_ERROR;
        }
      }
    } break;
    case OP_CLOSE_UPVALUE:
      close_upvalues(vm.stack_top - 1);
      pop();
      break;
    case OP_INHERIT: {
      value_t super_class = peek(1);
      if (!IS_CLASS(super_class)) {
        runtime_error("Superclass must be a class");
        return RESULT_RUNTIME_ERROR;
      }

      obj_class_t *sub_class = AS_CLASS(peek(0));
      table_add_all(&AS_CLASS(super_class)->methods, &sub_class->methods);
      pop();
    } break;
    case OP_ASSERT: {
      value_t value = pop();
      if (is_falsey(value)) {
        runtime_error("Assertion failed!");
        set_signal(SIG_ASSERT_FAIL, -1);
        return RESULT_RUNTIME_ERROR;
      }
    } break;
    case OP_ASSERT_MSG: {
      value_t msg = pop();
      value_t value = pop();
      if (is_falsey(value)) {
        runtime_error("Assertion failed with:");
        print_value(msg, false);
        printf("\n");
        set_signal(SIG_ASSERT_FAIL, -1);
        return RESULT_RUNTIME_ERROR;
      }
    } break;
    case OP_CALL: {
      unsigned int argc = READ_BYTE();
      if (!call_value(peek(argc), argc))
        return RESULT_RUNTIME_ERROR;
      UPDATE_FRAME();
    } break;
    case OP_LOOP: {
      uint16_t offset = READ_SHORT();
      frame->ip -= offset;
    } break;
    case OP_JUMP: {
      uint16_t offset = READ_SHORT();
      frame->ip += offset;
    } break;
    case OP_JUMP_IF_FALSE: {
      uint16_t offset = READ_SHORT();
      if (is_falsey(peek(0)))
        frame->ip += offset;
    } break;
    case OP_RETURN: {
      value_t result = pop();
      close_upvalues(frame->slots);
      bool is_module = frame->is_module;
      vm.frame_count--;
      if (vm.frame_count == 0) {
        pop();
        return RESULT_OK;
      }

      vm.stack_top = frame->slots;
      if (!is_module)
        push(result);
      UPDATE_FRAME();
    } break;
    }

    if (vm.signal != SIG_NONE)
      switch (vm.signal) {
      case SIG_NONE: // Unreachable
      case SIG_TEST_ASSERT_FAIL:
        break;
      case SIG_ASSERT_FAIL:
      case SIG_STACK_OVERFLOW:
      case SIG_STACK_UNDERFLOW:
      case SIG_RUNTIME_ERROR:
        return RESULT_RUNTIME_ERROR;
      case SIG_HALT:
        return RESULT_OK;
      }

    if (vm.update_frame) {
      vm.update_frame = false;
      UPDATE_FRAME();
    }
  }

#undef UPDATE_FRAME
#undef IS_NUM_OR_FLT

#undef READ_STRING_LONG
#undef READ_STRING
#undef READ_CONSTANT_LONG
#undef READ_CONSTANT
#undef READ_LONG
#undef READ_SHORT
#undef READ_BYTE
}

result_t interpret(const char *source) {
  obj_module_t *module = compile(source, NULL);
  if (module == NULL)
    return RESULT_COMPILE_ERROR;

  push(OBJ_VAL(module));
  call(module->init, 0);

  return run();
}
