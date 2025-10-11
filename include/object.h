#ifndef XYL_OBJECT_H
#define XYL_OBJECT_H

#include <stdbool.h>
#include <stdio.h>

#include "chunk.h"
#include "table.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_BOUND_METHOD(value) is_obj_type(value, OBJ_BOUND_METHOD)
#define IS_CLASS(value) is_obj_type(value, OBJ_CLASS)
#define IS_CLOSURE(value) is_obj_type(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) is_obj_type(value, OBJ_FUNCTION)
#define IS_INSTANCE(value) is_obj_type(value, OBJ_INSTANCE)
#define IS_BUILTIN(value) is_obj_type(value, OBJ_BUILTIN)
#define IS_STRING(value) is_obj_type(value, OBJ_STRING)
#define IS_VECTOR(value) is_obj_type(value, OBJ_VECTOR)
#define IS_LIST(value) is_obj_type(value, OBJ_LIST)
#define IS_ARRAY(value) is_obj_type(value, OBJ_ARRAY)
#define IS_FILE(value) is_obj_type(value, OBJ_FILE)
#define IS_MODULE(value) is_obj_type(value, OBJ_MODULE)
#define IS_RANGE(value) is_obj_type(value, OBJ_RANGE)
#define IS_RESULT(value) is_obj_type(value, OBJ_RESULT)
#define IS_ENUM(value) is_obj_type(value, OBJ_ENUM)

#define AS_BOUND_METHOD(value) ((obj_bound_method_t *)AS_OBJ(value))
#define AS_CLASS(value) ((obj_class_t *)AS_OBJ(value))
#define AS_CLOSURE(value) ((obj_closure_t *)AS_OBJ(value))
#define AS_FUNCTION(value) ((obj_function_t *)AS_OBJ(value))
#define AS_INSTANCE(value) ((obj_instance_t *)AS_OBJ(value))
#define AS_BUILTIN(value) (((obj_builtin_t *)AS_OBJ(value))->function)
#define AS_STRING(value) ((obj_string_t *)AS_OBJ(value))
#define AS_CSTRING(value) (((obj_string_t *)AS_OBJ(value))->chars)
#define AS_VECTOR(value) ((obj_vector_t *)AS_OBJ(value))
#define AS_LIST(value) ((obj_list_t *)AS_OBJ(value))
#define AS_ARRAY(value) ((obj_array_t *)AS_OBJ(value))
#define AS_FILE(value) ((obj_file_t *)AS_OBJ(value))
#define AS_MODULE(value) ((obj_module_t *)AS_OBJ(value))
#define AS_RANGE(value) ((obj_range_t *)AS_OBJ(value))
#define AS_RESULT(value) ((obj_result_t *)AS_OBJ(value))
#define AS_ENUM(value) ((obj_enum_t *)AS_OBJ(value))

typedef enum {
  OBJ_STRING,
  OBJ_VECTOR,
  OBJ_LIST,
  OBJ_ARRAY,
  OBJ_FILE,
  OBJ_RANGE,
  OBJ_RESULT,
  OBJ_CLASS,
  OBJ_BOUND_METHOD,
  OBJ_INSTANCE,
  OBJ_CLOSURE,
  OBJ_FUNCTION,
  OBJ_BUILTIN,
  OBJ_UPVALUE,
  OBJ_MODULE,
  OBJ_ENUM,
  OBJ_ANY,
} obj_type_t;

struct obj {
  obj_type_t type;
  bool is_marked;
  struct obj *next;
};

typedef struct {
  obj_t obj;
  int arity;
  int upvalue_count;
  chunk_t chunk;
  obj_string_t *name;
  obj_string_t *path;
  int row, col;
  table_t *globals;
  bool has_varargs;
} obj_function_t;

typedef value_t (*builtin_fn_t)(int argc, value_t *args);

typedef struct {
  obj_t obj;
  builtin_fn_t function;
} obj_builtin_t;

struct obj_string {
  obj_t obj;
  char *chars;
  uint32_t hash;
  int length;
  bool interned;
};

typedef struct obj_upvalue {
  obj_t obj;
  value_t *location;
  value_t closed;
  struct obj_upvalue *next;
} obj_upvalue_t;

typedef struct {
  obj_t obj;
  obj_function_t *function;
  obj_upvalue_t **upvalues;
  int upvalue_count;
} obj_closure_t;

typedef struct {
  obj_t obj;
  obj_string_t *name;
  table_t methods;
} obj_class_t;

typedef struct {
  obj_t obj;
  obj_class_t *clas;
  table_t fields;
} obj_instance_t;

typedef struct {
  obj_t obj;
  value_t receiver;
  obj_closure_t *method;
} obj_bound_method_t;

typedef struct {
  obj_t obj;
  int count;
  int capacity;
  value_t *values;
  bool spread;
} obj_vector_t;

typedef struct {
  obj_t obj;
  value_t *values;
  int count;
  bool spread;
} obj_list_t;

typedef struct {
  obj_t obj;
  value_t *values;
  int count;
} obj_array_t;

typedef struct {
  obj_t obj;
  FILE *file;
  bool open;
  bool readable;
  bool writable;
  bool can_close;
} obj_file_t;

typedef struct {
  obj_t obj;
  obj_string_t *name;
  table_t globals;
  obj_closure_t *init;
} obj_module_t;

typedef struct {
  obj_t obj;
  value_t from;
  value_t to;
} obj_range_t;

typedef struct {
  obj_t obj;
  bool is_ok;
  value_t value;
} obj_result_t;

typedef struct enum_member enum_member_t;
typedef struct {
  obj_t obj;
  obj_string_t *name;
  table_t values;
  int64_t last;
} obj_enum_t;

obj_bound_method_t *new_bound_method(value_t receiver, obj_closure_t *method);
obj_class_t *new_class(obj_string_t *name);
obj_closure_t *new_closure(obj_function_t *function);
obj_function_t *new_function(void);
obj_instance_t *new_instance(obj_class_t *clas);
obj_builtin_t *new_builtin(builtin_fn_t function);
obj_string_t *take_string(char *chars, int length);
obj_string_t *copy_string(const char *chars, int length, bool intern);
obj_upvalue_t *new_upvalue(value_t *slot);
obj_vector_t *new_vector(int initial_capacity);
obj_list_t *new_list(int count);
obj_array_t *new_array(int count);
obj_file_t *new_file(const char *path, const char *mode);
obj_module_t *new_module(obj_string_t *name);
obj_range_t *new_range(value_t from, value_t to);
obj_result_t *new_result_ok(value_t value);
obj_result_t *new_result_err(value_t error);
obj_enum_t *new_enum(obj_string_t *name);
void print_object(FILE *stream, value_t value, bool literally);

void add_enum_value(obj_enum_t *enum_, obj_string_t *name);

static inline bool is_obj_type(value_t value, obj_type_t type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
