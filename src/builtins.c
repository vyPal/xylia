#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

typedef struct {
  char *data;
  size_t capacity;
  size_t length;
} string_builder_t;

void sb_init(string_builder_t *sb) {
  sb->capacity = 64;
  sb->length = 0;
  sb->data = (char *)malloc(sb->capacity);
  sb->data[0] = '\0';
}

void sb_append(string_builder_t *sb, const char *str, size_t len) {
  while (sb->length + len + 1 > sb->capacity) {
    sb->capacity *= 2;
    sb->data = (char *)realloc(sb->data, sb->capacity);
  }

  memcpy(sb->data + sb->length, str, len);
  sb->length += len;
  sb->data[sb->length] = '\0';
}

void sb_free(string_builder_t *sb) {
  free(sb->data);
}

obj_string_t *value_to_string(value_t value, bool literal) {
  char buf[32];
  string_builder_t sb;

  switch (value.type) {
  case VAL_BOOL:
    return vm.vm_strings[AS_BOOL(value) ? VM_STR_TRUE : VM_STR_FALSE];
  case VAL_NIL:
    return vm.vm_strings[VM_STR_NIL];
  case VAL_NUMBER: {
    int64_t number = AS_NUMBER(value);
    int len = snprintf(buf, sizeof(buf), "%lld", (long long)number);
    return copy_string(buf, len, true);
  }
  case VAL_FLOAT: {
    double flt = AS_FLOAT(value);
    int len = snprintf(buf, sizeof(buf), "%g", flt);
    return copy_string(buf, len, true);
  }
  case VAL_OBJ: {
    switch (OBJ_TYPE(value)) {
    case OBJ_STRING: {
      if (literal) {
        obj_string_t *string = AS_STRING(value);
        sb_init(&sb);
        sb_append(&sb, "\"", 1);
        sb_append(&sb, string->chars, string->length);
        sb_append(&sb, "\"", 1);
        obj_string_t *res = copy_string(sb.data, sb.length, true);
        sb_free(&sb);
        return res;
      } else
        return AS_STRING(value);
    }
    case OBJ_VECTOR: {
      obj_vector_t *vector = AS_VECTOR(value);
      sb_init(&sb);
      sb_append(&sb, "{", 1);
      for (int i = 0; i < vector->count; i++) {
        if (i != 0)
          sb_append(&sb, ", ", 2);
        obj_string_t *str_val = value_to_string(vector->values[i], true);
        sb_append(&sb, str_val->chars, str_val->length);
      }
      sb_append(&sb, "}", 1);
      obj_string_t *res = copy_string(sb.data, sb.length, true);
      sb_free(&sb);
      return res;
    }
    case OBJ_LIST: {
      obj_list_t *list = AS_LIST(value);
      sb_init(&sb);
      sb_append(&sb, "[", 1);
      for (int i = 0; i < list->count; i++) {
        if (i != 0)
          sb_append(&sb, ", ", 2);
        obj_string_t *str_val = value_to_string(list->values[i], true);
        sb_append(&sb, str_val->chars, str_val->length);
      }
      sb_append(&sb, "]", 1);
      obj_string_t *res = copy_string(sb.data, sb.length, true);
      sb_free(&sb);
      return res;
    }
    case OBJ_FILE:
      return copy_string("<file>", 6, true);
    case OBJ_RANGE: {
      obj_range_t *range = AS_RANGE(value);
      sb_init(&sb);
      sb_append(&sb, "<range ", 7);
      obj_string_t *from = value_to_string(range->from, true);
      obj_string_t *to = value_to_string(range->to, true);
      sb_append(&sb, from->chars, from->length);
      sb_append(&sb, ":", 1);
      sb_append(&sb, to->chars, to->length);
      sb_append(&sb, ">", 1);
      obj_string_t *res = copy_string(sb.data, sb.length, true);
      sb_free(&sb);
      return res;
    }
    case OBJ_CLASS: {
      obj_class_t *clas = AS_CLASS(value);
      sb_init(&sb);
      sb_append(&sb, "<class ", 7);
      sb_append(&sb, clas->name->chars, clas->name->length);
      sb_append(&sb, ">", 1);
      obj_string_t *res = copy_string(sb.data, sb.length, true);
      sb_free(&sb);
      return res;
    }
    case OBJ_BOUND_METHOD: {
      obj_function_t *function = AS_BOUND_METHOD(value)->method->function;
      if (function->name == NULL)
        return copy_string("<script>", 8, true);
      sb_init(&sb);
      sb_append(&sb, "<fn ", 4);
      sb_append(&sb, function->name->chars, function->name->length);
      sb_append(&sb, ">", 1);
      obj_string_t *res = copy_string(sb.data, sb.length, true);
      sb_free(&sb);
      return res;
    }
    case OBJ_INSTANCE: {
      obj_instance_t *instance = AS_INSTANCE(value);
      sb_init(&sb);
      sb_append(&sb, "<instance ", 10);
      sb_append(&sb, instance->clas->name->chars, instance->clas->name->length);
      sb_append(&sb, ">", 1);
      obj_string_t *res = copy_string(sb.data, sb.length, true);
      sb_free(&sb);
      return res;
    }
    case OBJ_CLOSURE: {
      obj_function_t *function = AS_CLOSURE(value)->function;
      if (function->name == NULL)
        return copy_string("<script>", 8, true);
      sb_init(&sb);
      sb_append(&sb, "<fn ", 4);
      sb_append(&sb, function->name->chars, function->name->length);
      sb_append(&sb, ">", 1);
      obj_string_t *res = copy_string(sb.data, sb.length, true);
      sb_free(&sb);
      return res;
    }
    case OBJ_FUNCTION: {
      obj_function_t *function = AS_FUNCTION(value);
      if (function->name == NULL)
        return copy_string("<script>", 8, true);
      sb_init(&sb);
      sb_append(&sb, "<fn ", 4);
      sb_append(&sb, function->name->chars, function->name->length);
      sb_append(&sb, ">", 1);
      obj_string_t *res = copy_string(sb.data, sb.length, true);
      sb_free(&sb);
      return res;
    }
    case OBJ_BUILTIN:
      return copy_string("<fn builtin>", 12, true);
    case OBJ_UPVALUE:
      return copy_string("<upvalue>", 9, true);
    case OBJ_MODULE: {
      obj_module_t *module = AS_MODULE(value);
      sb_init(&sb);
      sb_append(&sb, "<module ", 8);
      sb_append(&sb, module->name->chars, module->name->length);
      sb_append(&sb, ">", 1);
      obj_string_t *res = copy_string(sb.data, sb.length, true);
      sb_free(&sb);
      return res;
    }
    case OBJ_ANY:
      break;
    }
  }
  case VAL_ANY:
    break;
  }

  return NULL;
}

typedef struct {
  value_type_t value_type;
  obj_type_t obj_type;
} builtin_arg_t;

typedef struct {
  const char *name;
  int argc;
  builtin_arg_t *args;
} builtin_signature_t;

typedef enum {
  ARGC_EXACT,
  ARGC_MORE_OR_EXACT,
  ARGC_LESS_OR_EXACT,
} argc_comparison_t;

static const char *value_type_to_str(value_type_t type) {
  switch (type) {
  case VAL_BOOL:
    return "bool";
  case VAL_NIL:
    return "nil";
  case VAL_NUMBER:
    return "number";
  case VAL_FLOAT:
    return "float";
  case VAL_OBJ:
    return "obj";
  case VAL_ANY:
    return "any";
  }
}

static const char *obj_type_to_str(obj_type_t type) {
  switch (type) {
  case OBJ_STRING:
    return "string";
  case OBJ_VECTOR:
    return "vector";
  case OBJ_LIST:
    return "list";
  case OBJ_FILE:
    return "file";
  case OBJ_CLASS:
    return "class";
  case OBJ_BOUND_METHOD:
  case OBJ_FUNCTION:
  case OBJ_CLOSURE:
    return "function";
  case OBJ_INSTANCE:
    return "instance";
  case OBJ_BUILTIN:
    return "builtin";
  case OBJ_UPVALUE:
    return "upvalue";
  case OBJ_MODULE:
    return "module";
  case OBJ_RANGE:
    return "range";
  case OBJ_ANY:
    return "any";
  }
}

static inline bool check_signature(const builtin_signature_t *signature,
                                   int argc, value_t *argv,
                                   argc_comparison_t comparison) {
  switch (comparison) {
  case ARGC_EXACT: {
    if (argc != signature->argc) {
      runtime_error("Expected %d %s in '%s' but got %d", signature->argc,
                    signature->argc == 1 ? "argument" : "arguments",
                    signature->name, argc);
      return false;
    }
  } break;
  case ARGC_MORE_OR_EXACT: {
    if (argc < signature->argc) {
      runtime_error("Expected at least %d %s in '%s' but got %d",
                    signature->argc,
                    signature->argc == 1 ? "argument" : "arguments",
                    signature->name, argc);
      return false;
    }
  } break;
  case ARGC_LESS_OR_EXACT: {
    if (argc > signature->argc) {
      runtime_error("Expected at most %d %s in '%s' but got %d",
                    signature->argc,
                    signature->argc == 1 ? "argument" : "arguments",
                    signature->name, argc);
      return false;
    }
  } break;
  }

  int checks = argc > signature->argc ? signature->argc : argc;
  for (int i = 0; i < checks; i++) {
    value_type_t want_type = signature->args[i].value_type;
    if (want_type == VAL_ANY)
      continue;

    value_type_t got_type = argv[i].type;
    if (got_type != want_type) {
      runtime_error("Expected argument %d in '%s' to be '%s' but got '%s'",
                    i + 1, signature->name, value_type_to_str(want_type),
                    value_type_to_str(got_type));
      return false;
    }

    if (want_type == VAL_OBJ) {
      obj_type_t want_obj = signature->args[i].obj_type;
      if (want_obj == OBJ_ANY)
        continue;

      obj_type_t got_obj = OBJ_TYPE(argv[i]);
      if (got_obj != want_obj) {
        runtime_error("Expected argument %d in '%s' to be '%s' but got '%s'",
                      i + 1, signature->name, obj_type_to_str(want_obj),
                      obj_type_to_str(got_obj));
        return false;
      }
    }
  }

  return true;
}

#define xyl_builtin_signature(name, arg_count, comparison, ...)                \
  const builtin_arg_t name##_args[] = {__VA_ARGS__};                           \
  const builtin_signature_t name##_signature = {#name, arg_count,              \
                                                (builtin_arg_t *)name##_args}; \
  if (!check_signature(&name##_signature, argc, argv, comparison))             \
  return NIL_VAL

#define xyl_check_signature(name, comparison)                                  \
  check_signature(&name##_signature, argc, argv, comparison)

xyl_builtin(print) {
  for (int i = 0; i < argc; i++) {
    if (i != 0)
      putchar(' ');
    print_value(argv[i], false);
  }
  return NIL_VAL;
}

xyl_builtin(println) {
  for (int i = 0; i < argc; i++) {
    if (i != 0)
      putchar(' ');
    print_value(argv[i], false);
  }
  putchar('\n');
  return NIL_VAL;
}

xyl_builtin(printf) {
  xyl_builtin_signature(printf, 1, ARGC_MORE_OR_EXACT, {VAL_OBJ, OBJ_STRING});

  obj_string_t *fmt = AS_STRING(argv[0]);
  int fmt_argc = 1;

  for (int i = 0; i < fmt->length; i++) {
    if (fmt->chars[i] == '%') {
      if (i + 1 < fmt->length && fmt->chars[i + 1] == '%')
        putchar(fmt->chars[i++]);
      else {
        if (fmt_argc >= argc) {
          runtime_error("Not enough arguments in printf");
          return NIL_VAL;
        }
        print_value(argv[fmt_argc++], false);
      }
    } else
      putchar(fmt->chars[i]);
  }

  return NIL_VAL;
}

xyl_builtin(input) {
  xyl_builtin_signature(input, 1, ARGC_LESS_OR_EXACT, {VAL_OBJ, OBJ_STRING});

  if (argc == 1)
    print_value(argv[0], false);

  size_t capacity = 64;
  size_t length = 0;
  char *buffer = (char *)malloc(capacity);

  if (!buffer) {
    runtime_error("[%s:%s] Failed to allocate memory for input buffer",
                  __FILE_NAME__, __PRETTY_FUNCTION__);
    return NIL_VAL;
  }

  int c;
  while ((c = fgetc(stdin)) != EOF) {
    if (length + 1 >= capacity) {
      capacity *= 2;
      char *new_buffer = realloc(buffer, capacity);
      if (!new_buffer) {
        runtime_error("[%s:%s] Failed to reallocate memory for input buffer",
                      __FILE_NAME__, __PRETTY_FUNCTION__);
        free(buffer);
        return NIL_VAL;
      }
      buffer = new_buffer;
    }

    if (c == '\n')
      break;

    buffer[length++] = (char)c;
  }

  if (length == 0 && c == EOF) {
    free(buffer);
    return NIL_VAL;
  }

  buffer[length] = '\0';
  obj_string_t *str = copy_string(buffer, length, false);
  free(buffer);

  return OBJ_VAL(str);
}

xyl_builtin(open) {
  xyl_builtin_signature(open, 2, ARGC_EXACT, {VAL_OBJ, OBJ_STRING},
                        {VAL_OBJ, OBJ_STRING});

  const char *path = AS_CSTRING(argv[0]);
  const char *mode = AS_CSTRING(argv[1]);
  obj_file_t *file = new_file(path, mode);

  return OBJ_VAL(file);
}

xyl_builtin(close) {
  xyl_builtin_signature(close, 1, ARGC_EXACT, {VAL_OBJ, OBJ_FILE});

  obj_file_t *file = AS_FILE(argv[0]);
  if (!file->open)
    return NIL_VAL;

  fclose(file->file);
  file->open = false;
  file->readable = false;
  file->writable = false;

  return NIL_VAL;
}

xyl_builtin(read) {
  xyl_builtin_signature(read, 1, ARGC_EXACT, {VAL_OBJ, OBJ_FILE});

  obj_file_t *file = AS_FILE(argv[0]);
  if (!file->open) {
    runtime_error("File is closed");
    return NIL_VAL;
  }

  if (!file->readable) {
    runtime_error("File is not readable");
    return NIL_VAL;
  }

  long start_pos = ftell(file->file);
  if (start_pos < 0) {
    runtime_error("Invalid file position");
    return NIL_VAL;
  }

  if (fseek(file->file, 0, SEEK_END) != 0) {
    runtime_error("Failed to determine the end of file");
    return NIL_VAL;
  }

  long file_size = ftell(file->file);
  if (file_size < 0) {
    runtime_error("Invalid file size");
    return NIL_VAL;
  }

  char *buffer = (char *)malloc(file_size + 1);
  if (!buffer) {
    runtime_error("[%s:%s] Failed to allocate memory for input buffer",
                  __FILE_NAME__, __PRETTY_FUNCTION__);
    return NIL_VAL;
  }

  if (fseek(file->file, 0, SEEK_SET) != 0) {
    runtime_error("Failed to read the file");
    free(buffer);
    return NIL_VAL;
  }

  size_t read_size = fread(buffer, 1, file_size, file->file);
  buffer[read_size] = '\0';

  fseek(file->file, start_pos, SEEK_SET);

  return OBJ_VAL(copy_string(buffer, read_size, false));
}

xyl_builtin(write) {
  xyl_builtin_signature(write, 2, ARGC_EXACT, {VAL_OBJ, OBJ_FILE},
                        {VAL_OBJ, OBJ_STRING});

  obj_file_t *file = AS_FILE(argv[0]);
  const char *data = AS_CSTRING(argv[1]);
  if (!file->open) {
    runtime_error("File is closed");
    return NIL_VAL;
  }

  if (!file->writable) {
    runtime_error("File is not writable");
    return NIL_VAL;
  }

  if (fputs(data, file->file) == EOF) {
    runtime_error("Failed to write to file");
    return NIL_VAL;
  }
  fflush(file->file);

  return NIL_VAL;
}

xyl_builtin(len) {
  xyl_builtin_signature(len, 1, ARGC_EXACT, {VAL_OBJ, OBJ_ANY});

  if (IS_STRING(argv[0]))
    return NUMBER_VAL(AS_STRING(argv[0])->length);
  else if (IS_VECTOR(argv[0]))
    return NUMBER_VAL(AS_VECTOR(argv[0])->count);
  else if (IS_LIST(argv[0]))
    return NUMBER_VAL(AS_LIST(argv[0])->count);

  runtime_error("Expected first argument in len to be string or vector");
  return NIL_VAL;
}

xyl_builtin(append) {
  xyl_builtin_signature(append, 2, ARGC_MORE_OR_EXACT, {VAL_OBJ, OBJ_VECTOR},
                        {VAL_ANY, OBJ_ANY});

  obj_vector_t *vector = AS_VECTOR(argv[0]);

  for (int i = 1; i < argc; i++) {
    if (IS_VECTOR(argv[i]) && AS_VECTOR(argv[i])->spread) {
      for (int j = 0; j < AS_VECTOR(argv[i])->count; j++) {
        if (vector->count >= vector->capacity) {
          int old_capacity = vector->capacity;
          vector->capacity = GROW_CAPACITY(old_capacity);
          vector->values = GROW_ARRAY(value_t, vector->values, old_capacity,
                                      vector->capacity);
        }

        vector->values[vector->count++] = AS_VECTOR(argv[i])->values[j];
      }
    } else if (IS_LIST(argv[i]) && AS_LIST(argv[i])->spread) {
      for (int j = 0; j < AS_LIST(argv[i])->count; j++) {
        if (vector->count >= vector->capacity) {
          int old_capacity = vector->capacity;
          vector->capacity = GROW_CAPACITY(old_capacity);
          vector->values = GROW_ARRAY(value_t, vector->values, old_capacity,
                                      vector->capacity);
        }

        vector->values[vector->count++] = AS_LIST(argv[i])->values[j];
      }
    } else {
      if (vector->count >= vector->capacity) {
        int old_capacity = vector->capacity;
        vector->capacity = GROW_CAPACITY(old_capacity);
        vector->values =
            GROW_ARRAY(value_t, vector->values, old_capacity, vector->capacity);
      }

      vector->values[vector->count++] = argv[i];
    }
  }

  return NIL_VAL;
}

xyl_builtin(pop) {
  xyl_builtin_signature(pop, 1, ARGC_EXACT, {VAL_OBJ, OBJ_VECTOR});

  obj_vector_t *vector = AS_VECTOR(argv[0]);
  if (vector->count == 0) {
    runtime_error("pop called on empty vector");
    return NIL_VAL;
  }

  value_t value = vector->values[--vector->count];
  return value;
}

xyl_builtin(insert) {
  xyl_builtin_signature(insert, 3, ARGC_EXACT, {VAL_OBJ, OBJ_VECTOR},
                        {VAL_NUMBER, OBJ_ANY}, {VAL_ANY, OBJ_ANY});

  obj_vector_t *vector = AS_VECTOR(argv[0]);
  int64_t index = AS_NUMBER(argv[1]);
  value_t value = argv[2];

  if (index < 0 || index > vector->count) {
    runtime_error("Index %d out of range", index);
    return NIL_VAL;
  }

  if (vector->count >= vector->capacity) {
    int old_capacity = vector->capacity;
    vector->capacity = GROW_CAPACITY(old_capacity);
    vector->values =
        GROW_ARRAY(value_t, vector->values, old_capacity, vector->capacity);
  }

  for (int i = vector->count; i > index; i--)
    vector->values[i] = vector->values[i - 1];

  vector->values[index] = value;
  vector->count++;
  return NIL_VAL;
}

xyl_builtin(remove) {
  xyl_builtin_signature(remove, 2, ARGC_EXACT, {VAL_OBJ, OBJ_VECTOR},
                        {VAL_NUMBER, OBJ_ANY});

  obj_vector_t *vector = AS_VECTOR(argv[0]);
  int64_t index = AS_NUMBER(argv[1]);

  if (index < 0 || index >= vector->count) {
    runtime_error("Index %d out of range", index);
    return NIL_VAL;
  }

  value_t removed = vector->values[index];
  vector->count--;

  for (int i = index; i < vector->count; i++)
    vector->values[i] = vector->values[i + 1];

  return removed;
}

xyl_builtin(slice) {
  xyl_builtin_signature(slice, 3, ARGC_EXACT, {VAL_OBJ, OBJ_ANY},
                        {VAL_NUMBER, OBJ_ANY}, {VAL_NUMBER, OBJ_ANY});

  if (IS_VECTOR(argv[0])) {
    obj_vector_t *vector = AS_VECTOR(argv[0]);
    int64_t from = AS_NUMBER(argv[1]);
    int64_t to = AS_NUMBER(argv[2]);

    if (from > to) {
      runtime_error("Start index can not be bigger than end index");
      return NIL_VAL;
    }

    if (from < 0 || from > vector->count) {
      runtime_error("Index %d out of range", from);
      return NIL_VAL;
    }

    if (to < 0 || to > vector->count) {
      runtime_error("Index %d out of range", to);
      return NIL_VAL;
    }

    if (from == to)
      return OBJ_VAL(new_vector(4));

    obj_vector_t *new_vec = new_vector(to - from);
    for (int i = 0; i < (to - from); i++)
      new_vec->values[new_vec->count++] = vector->values[from + i];

    return OBJ_VAL(new_vec);
  } else if (IS_LIST(argv[0])) {
    obj_list_t *list = AS_LIST(argv[0]);
    int64_t from = AS_NUMBER(argv[1]);
    int64_t to = AS_NUMBER(argv[2]);

    if (from > to) {
      runtime_error("Start index can not be bigger than end index");
      return NIL_VAL;
    }

    if (from < 0 || from > list->count) {
      runtime_error("Index %d out of range", from);
      return NIL_VAL;
    }

    if (to < 0 || to > list->count) {
      runtime_error("Index %d out of range", to);
      return NIL_VAL;
    }

    if (from == to) {
      runtime_error("Can not create empty list slice");
      return NIL_VAL;
    }

    obj_list_t *new_lst = new_list(to - from);
    for (int i = 0; i < (to - from); i++)
      new_lst->values[i] = list->values[from + i];

    return OBJ_VAL(new_lst);
  } else if (IS_STRING(argv[0])) {
    obj_string_t *string = AS_STRING(argv[0]);
    int64_t from = AS_NUMBER(argv[1]);
    int64_t to = AS_NUMBER(argv[2]);

    if (from > to) {
      runtime_error("Start index can not be bigger than end index");
      return NIL_VAL;
    }

    if (from < 0 || from > string->length) {
      runtime_error("Index %d out of range", from);
      return NIL_VAL;
    }

    if (to < 0 || to > string->length) {
      runtime_error("Index %d out of range", to);
      return NIL_VAL;
    }

    if (from == to)
      return OBJ_VAL(copy_string("", 0, true));

    obj_string_t *new_str = copy_string(string->chars + from, to - from, true);
    return OBJ_VAL(new_str);
  }

  runtime_error("Can call slice only on vecor, list and string");
  return NIL_VAL;
}

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

xyl_builtin(string) {
  xyl_builtin_signature(string, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  obj_string_t *stringified = value_to_string(argv[0], false);
  if (stringified == NULL)
    return NIL_VAL;
  return OBJ_VAL(stringified);
}

xyl_builtin(number) {
  xyl_builtin_signature(number, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});

  value_t arg = argv[0];
  switch (arg.type) {
  case VAL_BOOL:
    return NUMBER_VAL(AS_BOOL(arg) ? 1 : 0);
  case VAL_NIL:
    runtime_error("Can not cast 'nil' to 'number'");
    break;
  case VAL_NUMBER:
    return arg;
  case VAL_FLOAT:
    return NUMBER_VAL((int64_t)AS_FLOAT(arg));
  case VAL_OBJ:
    switch (OBJ_TYPE(arg)) {
    case OBJ_STRING: {
      const char *str = AS_CSTRING(arg);
      char *end;
      errno = 0;
      long long number = strtoll(str, &end, 10);
      if (errno == ERANGE || number > INT64_MAX || number < INT64_MIN ||
          *end != '\0') {
        runtime_error("Could not cast '%s' to 'number'", str);
        break;
      }
      return NUMBER_VAL((int64_t)number);
    }
    default:
      runtime_error("Can not cast '%s' to 'number'",
                    obj_type_to_str(OBJ_TYPE(arg)));
      break;
    }
  case VAL_ANY:
    break;
  }

  return NIL_VAL;
}

xyl_builtin(float) {
  xyl_builtin_signature(float, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});

  value_t arg = argv[0];
  switch (arg.type) {
  case VAL_BOOL:
    runtime_error("Can not cast 'bool' to 'float'");
    break;
  case VAL_NIL:
    runtime_error("Can not cast 'nil' to 'float'");
    break;
  case VAL_NUMBER:
    return FLOAT_VAL((double)AS_NUMBER(arg));
  case VAL_FLOAT:
    return arg;
  case VAL_OBJ:
    switch (OBJ_TYPE(arg)) {
    case OBJ_STRING: {
      const char *str = AS_CSTRING(arg);
      char *end;
      errno = 0;
      double flt = strtod(str, &end);
      if (errno == ERANGE || isnan(flt) || isinf(flt) || *end != '\0') {
        runtime_error("Could not cast '%s' to 'float'", str);
        break;
      }
      return FLOAT_VAL(flt);
    }
    default:
      runtime_error("Can not cast '%s' to 'float'",
                    obj_type_to_str(OBJ_TYPE(arg)));
      break;
    }
  case VAL_ANY:
    break;
  }

  return NIL_VAL;
}

xyl_builtin(bool) {
  xyl_builtin_signature(bool, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});

  value_t arg = argv[0];
  switch (arg.type) {
  case VAL_BOOL:
    return arg;
  case VAL_NIL:
    return BOOL_VAL(false);
  case VAL_NUMBER:
    return BOOL_VAL(AS_NUMBER(arg) == 0 ? false : true);
  case VAL_FLOAT:
    return BOOL_VAL(AS_FLOAT(arg) == 0.f ? false : true);
  case VAL_OBJ:
    switch (OBJ_TYPE(arg)) {
    case OBJ_STRING: {
      obj_string_t *str = AS_STRING(arg);
      if (str->length == 4 && strncmp(str->chars, "true", 4) == 0)
        return BOOL_VAL(true);
      else if (str->length == 5 && strncmp(str->chars, "false", 5) == 0)
        return BOOL_VAL(false);
      runtime_error("Could not cast '%s' to 'bool'", str->chars);
    }
    default:
      runtime_error("Can not cast '%s' to 'bool'",
                    obj_type_to_str(OBJ_TYPE(arg)));
      break;
    }
  case VAL_ANY:
    break;
  }

  return NIL_VAL;
}

xyl_builtin(vector) {
  xyl_builtin_signature(vector, 1, ARGC_EXACT, {VAL_OBJ, OBJ_ANY});

  value_t arg = argv[0];

  if (IS_VECTOR(arg))
    return arg;

  if (IS_LIST(arg)) {
    obj_list_t *list = AS_LIST(arg);
    obj_vector_t *vector = new_vector(list->count);
    vector->count = list->count;
    for (int i = 0; i < list->count; i++)
      vector->values[i] = list->values[i];
    return OBJ_VAL(vector);
  } else if (IS_RANGE(arg)) {
    obj_range_t *range = AS_RANGE(arg);
    if (!IS_NUMBER(range->from) || !IS_NUMBER(range->to)) {
      runtime_error("Range must be 'number':'number' but got '%s':'%s'",
                    value_type_to_str(range->from.type),
                    value_type_to_str(range->to.type));
      return NIL_VAL;
    }

    int64_t from = AS_NUMBER(range->from);
    int64_t to = AS_NUMBER(range->to);

    int64_t length = labs(to - from);
    obj_vector_t *vector = new_vector(length);
    vector->count = length;

    if (from < to) {
      for (int64_t i = 0; i < length; i++)
        vector->values[i] = NUMBER_VAL(from + i);
    } else if (from > to) {
      for (int64_t i = 0; i < length; i++)
        vector->values[i] = NUMBER_VAL(from - i);
    }

    return OBJ_VAL(vector);
  }

  runtime_error("Expected argument 1 in 'vector' to be 'list', 'vector' or "
                "'range' but got '%s'",
                obj_type_to_str(OBJ_TYPE(arg)));

  return NIL_VAL;
}

xyl_builtin(list) {
  xyl_builtin_signature(list, 1, ARGC_EXACT, {VAL_OBJ, OBJ_ANY});

  value_t arg = argv[0];

  if (IS_LIST(arg))
    return arg;

  if (IS_VECTOR(arg)) {
    obj_vector_t *vector = AS_VECTOR(arg);
    obj_list_t *list = new_list(vector->count);
    for (int i = 0; i < vector->count; i++)
      list->values[i] = vector->values[i];
    return OBJ_VAL(list);
  } else if (IS_RANGE(arg)) {
    obj_range_t *range = AS_RANGE(arg);
    if (!IS_NUMBER(range->from) || !IS_NUMBER(range->to)) {
      runtime_error("Range must be 'number':'number' but got '%s':'%s'",
                    value_type_to_str(range->from.type),
                    value_type_to_str(range->to.type));
      return NIL_VAL;
    }

    int64_t from = AS_NUMBER(range->from);
    int64_t to = AS_NUMBER(range->to);

    int64_t length = labs(to - from);
    obj_list_t *list = new_list(length);

    if (from < to) {
      for (int64_t i = 0; i < length; i++)
        list->values[i] = NUMBER_VAL(from + i);
    } else if (from > to) {
      for (int64_t i = 0; i < length; i++)
        list->values[i] = NUMBER_VAL(from - i);
    }

    return OBJ_VAL(list);
  }

  runtime_error("Expected argument 1 in 'list' to be 'vector', 'list' or "
                "'range' but got '%s'",
                obj_type_to_str(OBJ_TYPE(arg)));

  return NIL_VAL;
}

xyl_builtin(case_failed) {
  xyl_builtin_signature(case_failed, 0, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  bool failed = vm.signal == SIG_TEST_ASSERT_FAIL;
  vm.signal = SIG_NONE;
  return BOOL_VAL(failed);
}

xyl_builtin(assert_true) {
  xyl_builtin_signature(assert_true, 1, ARGC_EXACT, {VAL_BOOL, OBJ_ANY});
  if (!AS_BOOL(argv[0]))
    vm.signal = SIG_TEST_ASSERT_FAIL;
  return NIL_VAL;
}

xyl_builtin(assert_false) {
  xyl_builtin_signature(assert_false, 1, ARGC_EXACT, {VAL_BOOL, OBJ_ANY});
  if (AS_BOOL(argv[0]))
    vm.signal = SIG_TEST_ASSERT_FAIL;
  return NIL_VAL;
}

xyl_builtin(assert_eq) {
  xyl_builtin_signature(assert_eq, 2, ARGC_EXACT, {VAL_ANY, OBJ_ANY},
                        {VAL_ANY, OBJ_ANY});
  if (!values_equal(argv[0], argv[1]))
    vm.signal = SIG_TEST_ASSERT_FAIL;
  return NIL_VAL;
}

xyl_builtin(assert_neq) {
  xyl_builtin_signature(assert_neq, 2, ARGC_EXACT, {VAL_ANY, OBJ_ANY},
                        {VAL_ANY, OBJ_ANY});
  if (values_equal(argv[0], argv[1]))
    vm.signal = SIG_TEST_ASSERT_FAIL;
  return NIL_VAL;
}
