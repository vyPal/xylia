#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "object.h"
#include "vm.h"

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
    case OBJ_ARRAY: {
      obj_array_t *array = AS_ARRAY(value);
      sb_init(&sb);
      sb_append(&sb, "<", 1);
      for (int i = 0; i < array->count; i++) {
        if (i != 0)
          sb_append(&sb, ", ", 2);
        obj_string_t *str_val = value_to_string(array->values[i], true);
        sb_append(&sb, str_val->chars, str_val->length);
      }
      sb_append(&sb, ">", 1);
      obj_string_t *res = copy_string(sb.data, sb.length, true);
      sb_free(&sb);
      return res;
    } break;
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
    case OBJ_RESULT: {
      obj_result_t *result = AS_RESULT(value);
      sb_init(&sb);
      if (result->is_ok) {
        sb_append(&sb, "Ok(", 3);
      } else {
        sb_append(&sb, "Err(", 4);
      }
      obj_string_t *val = value_to_string(result->value, true);
      sb_append(&sb, val->chars, val->length);
      sb_append(&sb, ")", 1);
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

const char *value_type_to_str(value_type_t type) {
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

const char *obj_type_to_str(obj_type_t type) {
  switch (type) {
  case OBJ_STRING:
    return "string";
  case OBJ_VECTOR:
    return "vector";
  case OBJ_LIST:
    return "list";
  case OBJ_ARRAY:
    return "array";
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
  case OBJ_RESULT:
    return "result";
  case OBJ_ANY:
    return "any";
  }
}

bool check_signature(const builtin_signature_t *signature, int argc,
                     value_t *argv, argc_comparison_t comparison) {
  switch (comparison) {
  case ARGC_EXACT: {
    if (argc != signature->argc) {
      runtime_error(-1, "Expected %d %s in '%s' but got %d", signature->argc,
                    signature->argc == 1 ? "argument" : "arguments",
                    signature->name, argc);
      return false;
    }
  } break;
  case ARGC_MORE_OR_EXACT: {
    if (argc < signature->argc) {
      runtime_error(-1, "Expected at least %d %s in '%s' but got %d",
                    signature->argc,
                    signature->argc == 1 ? "argument" : "arguments",
                    signature->name, argc);
      return false;
    }
  } break;
  case ARGC_LESS_OR_EXACT: {
    if (argc > signature->argc) {
      runtime_error(-1, "Expected at most %d %s in '%s' but got %d",
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
      runtime_error(-1, "Expected argument %d in '%s' to be '%s' but got '%s'",
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
        runtime_error(-1,
                      "Expected argument %d in '%s' to be '%s' but got '%s'",
                      i + 1, signature->name, obj_type_to_str(want_obj),
                      obj_type_to_str(got_obj));
        return false;
      }
    }
  }

  return true;
}
