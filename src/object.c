#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, object_type)                                        \
  (type *)allocate_object(sizeof(type), object_type)

static obj_t *allocate_object(size_t size, obj_type_t type) {
  obj_t *object = (obj_t *)reallocate(NULL, 0, size);
  object->type = type;
  object->is_marked = false;
  object->next = vm.objects;
  vm.objects = object;
  return object;
}

obj_bound_method_t *new_bound_method(value_t receiver, obj_closure_t *method) {
  obj_bound_method_t *bound =
      ALLOCATE_OBJ(obj_bound_method_t, OBJ_BOUND_METHOD);
  bound->receiver = receiver;
  bound->method = method;
  return bound;
}

obj_class_t *new_class(obj_string_t *name) {
  obj_class_t *clas = ALLOCATE_OBJ(obj_class_t, OBJ_CLASS);
  clas->name = name;
  init_table(&clas->methods);
  return clas;
}

obj_closure_t *new_closure(obj_function_t *function) {
  obj_upvalue_t **upvalues = ALLOCATE(obj_upvalue_t *, function->upvalue_count);
  for (int i = 0; i < function->upvalue_count; i++)
    upvalues[i] = NULL;

  obj_closure_t *closure = ALLOCATE_OBJ(obj_closure_t, OBJ_CLOSURE);
  closure->function = function;
  closure->upvalues = upvalues;
  closure->upvalue_count = function->upvalue_count;
  return closure;
}

obj_function_t *new_function(void) {
  obj_function_t *function = ALLOCATE_OBJ(obj_function_t, OBJ_FUNCTION);
  function->arity = 0;
  function->upvalue_count = 0;
  function->name = NULL;
  function->globals = NULL;
  function->has_varargs = false;
  init_chunk(&function->chunk);
  return function;
}

obj_instance_t *new_instance(obj_class_t *clas) {
  obj_instance_t *instance = ALLOCATE_OBJ(obj_instance_t, OBJ_INSTANCE);
  instance->clas = clas;
  init_table(&instance->fields);
  return instance;
}

obj_builtin_t *new_builtin(builtin_fn_t function) {
  obj_builtin_t *builtin = ALLOCATE_OBJ(obj_builtin_t, OBJ_BUILTIN);
  builtin->function = function;
  return builtin;
}

static obj_string_t *allocate_string(char *chars, int length, uint32_t hash) {
  obj_string_t *string = ALLOCATE_OBJ(obj_string_t, OBJ_STRING);
  string->length = length;
  string->chars = chars;
  string->hash = hash;
  string->interned = true;

  push(OBJ_VAL(string));
  table_set(&vm.strings, string, NIL_VAL);
  pop();

  return string;
}

static uint32_t hash_string(const char *key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

obj_string_t *take_string(char *chars, int length) {
  uint32_t hash = hash_string(chars, length);
  obj_string_t *interned = table_find_string(&vm.strings, chars, length, hash);
  if (interned != NULL) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }

  return allocate_string(chars, length, hash);
}

obj_string_t *copy_string(const char *chars, int length, bool intern) {
  if (intern) {
    uint32_t hash = hash_string(chars, length);

    obj_string_t *interned =
        table_find_string(&vm.strings, chars, length, hash);
    if (interned != NULL)
      return interned;

    char *heap_chars = ALLOCATE(char, length + 1);
    memcpy(heap_chars, chars, length);
    heap_chars[length] = '\0';

    return allocate_string(heap_chars, length, hash);
  } else {
    obj_string_t *string = ALLOCATE_OBJ(obj_string_t, OBJ_STRING);
    string->length = length;
    string->chars = (char *)chars;
    string->interned = false;
    return string;
  }
}

obj_upvalue_t *new_upvalue(value_t *slot) {
  obj_upvalue_t *upvalue = ALLOCATE_OBJ(obj_upvalue_t, OBJ_UPVALUE);
  upvalue->location = slot;
  upvalue->closed = NIL_VAL;
  upvalue->next = NULL;
  return upvalue;
}

obj_vector_t *new_vector(int initial_capacity) {
  obj_vector_t *vector = ALLOCATE_OBJ(obj_vector_t, OBJ_VECTOR);
  vector->count = 0;
  vector->capacity = initial_capacity > 0 ? initial_capacity : 4;
  vector->spread = false;

  push(OBJ_VAL(vector));
  vector->values = ALLOCATE(value_t, vector->capacity);
  pop();
  return vector;
}

obj_list_t *new_list(int count) {
  obj_list_t *list = ALLOCATE_OBJ(obj_list_t, OBJ_LIST);
  list->values = NULL;
  list->count = count;
  list->spread = false;

  push(OBJ_VAL(list));
  if (count != 0)
    list->values = ALLOCATE(value_t, count);
  pop();
  return list;
}

obj_array_t *new_array(int count) {
  obj_array_t *array = ALLOCATE_OBJ(obj_array_t, OBJ_ARRAY);
  array->values = NULL;
  array->count = count;

  push(OBJ_VAL(array));
  if (count != 0)
    array->values = ALLOCATE(value_t, count);
  pop();

  for (int i = 0; i < count; i++)
    array->values[i] = NIL_VAL;
  return array;
}

obj_file_t *new_file(const char *path, const char *mode) {
  obj_file_t *file = ALLOCATE_OBJ(obj_file_t, OBJ_FILE);
  file->file = fopen(path, mode);
  file->open = false;
  file->readable = false;
  file->writable = false;
  file->can_close = true;

  if (!file->file) {
    runtime_error(-1, "Failed to open file '%s'", path);
    return file;
  }

  file->open = true;
  for (const char *p = mode; *p; p++) {
    switch (*p) {
    case 'r':
      file->readable = true;
      break;
    case 'w':
    case 'a':
      file->writable = true;
      break;
    case '+':
      file->readable = true;
      file->writable = true;
      break;
    }
  }

  return file;
}

obj_module_t *new_module(obj_string_t *name) {
  obj_module_t *module = ALLOCATE_OBJ(obj_module_t, OBJ_MODULE);
  module->name = name;
  module->init = NULL;
  push(OBJ_VAL(module));
  init_table(&module->globals);
  pop();
  return module;
}

obj_range_t *new_range(value_t from, value_t to) {
  obj_range_t *range = ALLOCATE_OBJ(obj_range_t, OBJ_RANGE);
  range->from = from;
  range->to = to;
  return range;
}

obj_result_t *new_result_ok(value_t value) {
  obj_result_t *result = ALLOCATE_OBJ(obj_result_t, OBJ_RESULT);
  result->is_ok = true;
  result->value = value;
  return result;
}

obj_result_t *new_result_err(value_t error) {
  obj_result_t *result = ALLOCATE_OBJ(obj_result_t, OBJ_RESULT);
  result->is_ok = false;
  result->value = error;
  return result;
}

obj_enum_t *new_enum(obj_string_t *name) {
  obj_enum_t *enum_ = ALLOCATE_OBJ(obj_enum_t, OBJ_ENUM);
  enum_->name = name;
  init_table(&enum_->values);
  enum_->last = -1;
  return enum_;
}

void add_enum_value(obj_enum_t *enum_, obj_string_t *name) {
  if (!table_set(&enum_->values, name, NUMBER_VAL(++enum_->last))) {
    runtime_error(-1, "Duplicate key in enum '%s'", enum_->name->chars);
    return;
  }
}

static void print_function(FILE *stream, obj_function_t *function) {
  if (function->name == NULL)
    fputs("<script>", stream);
  else
    fprintf(stream, "<fn %s>", function->name->chars);
}

static void print_vector(FILE *stream, obj_vector_t *vector) {
  fputc('{', stream);
  for (int i = 0; i < vector->count; i++) {
    if (i != 0)
      fputs(", ", stream);
    print_value(stream, vector->values[i], true);
  }
  fputc('}', stream);
}

static void print_list(FILE *stream, obj_list_t *list) {
  fputc('[', stream);
  for (int i = 0; i < list->count; i++) {
    if (i != 0)
      fputs(", ", stream);
    print_value(stream, list->values[i], true);
  }
  fputc(']', stream);
}

static void print_array(FILE *stream, obj_array_t *array) {
  fputc('<', stream);
  for (int i = 0; i < array->count; i++) {
    if (i != 0)
      fputs(", ", stream);
    print_value(stream, array->values[i], true);
  }
  fputc('>', stream);
}

static void print_literal(FILE *stream, const char *s) {
  fputc('"', stream);
  for (; *s; s++)
    switch (*s) {
    case '\n':
      fputs("\\n", stream);
      break;
    case '\t':
      fputs("\\t", stream);
      break;
    case '\r':
      fputs("\\r", stream);
      break;
    case '\v':
      fputs("\\v", stream);
      break;
    case '\f':
      fputs("\\f", stream);
      break;
    case '\b':
      fputs("\\b", stream);
      break;
    case '\\':
      fputs("\\\\", stream);
      break;
    case '\"':
      fputs("\\\"", stream);
      break;
    default:
      if ((unsigned char)*s < 32 || (unsigned char)*s == 127)
        fprintf(stream, "\\x%02X", (unsigned char)*s);
      else
        fputc(*s, stream);
    }
  fputc('"', stream);
}

void print_object(FILE *stream, value_t value, bool literally) {
  switch (OBJ_TYPE(value)) {
  case OBJ_BOUND_METHOD:
    print_function(stream, AS_BOUND_METHOD(value)->method->function);
    break;
  case OBJ_CLASS:
    fprintf(stream, "<class %s>", AS_CLASS(value)->name->chars);
    break;
  case OBJ_CLOSURE:
    print_function(stream, AS_CLOSURE(value)->function);
    break;
  case OBJ_FUNCTION:
    print_function(stream, AS_FUNCTION(value));
    break;
  case OBJ_INSTANCE:
    fprintf(stream, "<instance %s>", AS_INSTANCE(value)->clas->name->chars);
    break;
  case OBJ_BUILTIN:
    fputs("<fn builtin>", stream);
    break;
  case OBJ_STRING:
    if (literally)
      print_literal(stream, AS_CSTRING(value));
    else
      fprintf(stream, "%s", AS_CSTRING(value));
    break;
  case OBJ_UPVALUE:
    fputs("<upvalue>", stream);
    break;
  case OBJ_VECTOR:
    print_vector(stream, AS_VECTOR(value));
    break;
  case OBJ_LIST:
    print_list(stream, AS_LIST(value));
    break;
  case OBJ_ARRAY:
    print_array(stream, AS_ARRAY(value));
    break;
  case OBJ_FILE:
    fputs("<file>", stream);
    break;
  case OBJ_MODULE:
    fprintf(stream, "<module %s>", AS_MODULE(value)->name->chars);
    break;
  case OBJ_RANGE:
    fputs("<range ", stream);
    print_value(stream, AS_RANGE(value)->from, true);
    fputs(":", stream);
    print_value(stream, AS_RANGE(value)->to, true);
    fputs(">", stream);
    break;
  case OBJ_RESULT:
    if (AS_RESULT(value)->is_ok) {
      fputs("Ok(", stream);
      print_value(stream, AS_RESULT(value)->value, true);
      fputs(")", stream);
    } else {
      fputs("Err(", stream);
      print_value(stream, AS_RESULT(value)->value, true);
      fputs(")", stream);
    }
    break;
  case OBJ_ANY:
    break;
  }
}
