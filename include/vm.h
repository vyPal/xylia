#ifndef XYL_VM_H
#define XYL_VM_H

#include <stddef.h>

#include "object.h"
#include "table.h"
#include "value.h"

typedef enum {
  VM_STR_INIT,
  VM_STR_BOOL,
  VM_STR_NIL,
  VM_STR_NUMBER,
  VM_STR_FLOAT,
  VM_STR_FUNCTION,
  VM_STR_CLASS,
  VM_STR_CLOSURE,
  VM_STR_INSTANCE,
  VM_STR_BUILTIN,
  VM_STR_STRING,
  VM_STR_UPVALUE,
  VM_STR_VECTOR,
  VM_STR_LIST,
  VM_STR_ARRAY,
  VM_STR_FILE,
  VM_STR_NAN,
  VM_STR_MODULE,
  VM_STR_RANGE,
  VM_STR_TRUE,
  VM_STR_FALSE,
  VM_STR_OVERLOAD_EQ,        // ==
  VM_STR_OVERLOAD_GT,        // >
  VM_STR_OVERLOAD_GE,        // >=
  VM_STR_OVERLOAD_LT,        // <
  VM_STR_OVERLOAD_LE,        // <=
  VM_STR_OVERLOAD_ADD,       // +
  VM_STR_OVERLOAD_SUB,       // -
  VM_STR_OVERLOAD_MUL,       // *
  VM_STR_OVERLOAD_DIV,       // /
  VM_STR_OVERLOAD_NEG,       // -
  VM_STR_OVERLOAD_MOD,       // %
  VM_STR_OVERLOAD_XOR,       // ^
  VM_STR_OVERLOAD_BIT_OR,    // |
  VM_STR_OVERLOAD_BIT_AND,   // &
  VM_STR_OVERLOAD_BIT_NOT,   // ~
  VM_STR_OVERLOAD_LOG_NOT,   // !
  VM_STR_OVERLOAD_SET_INDEX, // []=
  VM_STR_OVERLOAD_GET_INDEX, // []
  VM_STR_OVERLOAD_SET_SLICE, // [:]=
  VM_STR_OVERLOAD_GET_SLICE, // [:]
  VM_STR_MAX,
} vm_strings_t;

typedef enum {
  SIG_NONE,
  SIG_STACK_OVERFLOW,
  SIG_STACK_UNDERFLOW,
  SIG_TEST_ASSERT_FAIL,
  SIG_ASSERT_FAIL,
  SIG_RUNTIME_ERROR,
  SIG_HALT,
} vm_singal_t;

typedef struct {
  obj_closure_t *closure;
  uint8_t *ip;
  value_t *slots;
  table_t *globals;
  bool is_module;
} call_frame_t;

typedef struct {
  call_frame_t *frames;
  int frame_capacity;
  int frame_count;
  bool update_frame;

  value_t *stack;
  value_t *stack_top;
  int stack_capacity;

  table_t module_lookup;
  table_t builtins;
  table_t strings;

  table_t *globals;

  obj_string_t *vm_strings[VM_STR_MAX];
  obj_list_t *args;

  obj_upvalue_t *open_upvalues;

  size_t bytes_allocated;
  size_t next_gc;
  obj_t *objects;

  obj_t **gray_stack;
  int gray_capacity;
  int gray_count;

  int offset;

  vm_singal_t signal;
  int exit_code;
} vm_t;

typedef enum {
  RESULT_OK,
  RESULT_COMPILE_ERROR,
  RESULT_RUNTIME_ERROR,
} result_t;

extern vm_t vm;

void set_signal(vm_singal_t sig, int exit_code);
void runtime_error(int offset, const char *fmt, ...);
void init_vm(void);
void free_vm(void);
void set_args(int argc, char **argv);
void load_test_functions(void);
result_t interpret(const char *source, const char *file);
void push(value_t value);
void push_frame(obj_closure_t *closure, int argc);
value_t pop(void);

#endif
