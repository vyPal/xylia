#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "vm.h"

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
