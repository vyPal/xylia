#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"

void init_value_array(value_array_t *array) {
  array->capacity = 0;
  array->count = 0;
  array->values = NULL;
}

void write_value_array(value_array_t *array, value_t value) {
  if (array->count >= array->capacity) {
    int old_capacity = array->capacity;
    array->capacity = GROW_CAPACITY(old_capacity);
    array->values =
        GROW_ARRAY(value_t, array->values, old_capacity, array->capacity);
  }

  array->values[array->count++] = value;
}

void free_value_array(value_array_t *array) {
  FREE_ARRAY(value_t, array->values, array->capacity);
  init_value_array(array);
}

void print_value(value_t value, bool literally) {
  switch (value.type) {
  case VAL_BOOL:
    printf(AS_BOOL(value) ? "true" : "false");
    break;
  case VAL_NIL:
    printf("nil");
    break;
  case VAL_NUMBER:
    printf("%ld", AS_NUMBER(value));
    break;
  case VAL_FLOAT:
    printf("%g", AS_FLOAT(value));
    break;
  case VAL_OBJ:
    print_object(value, literally);
    break;
  case VAL_ANY:
    break;
  }
}

bool values_equal(value_t a, value_t b) {
  if (IS_NUMBER(a) && IS_FLOAT(b))
    return (double)AS_NUMBER(a) == AS_FLOAT(b);
  if (IS_FLOAT(a) && IS_NUMBER(b))
    return AS_FLOAT(a) == (double)AS_NUMBER(b);

  if (a.type != b.type)
    return false;

  switch (a.type) {
  case VAL_BOOL:
    return AS_BOOL(a) == AS_BOOL(b);
  case VAL_NIL:
    return true;
  case VAL_NUMBER:
    return AS_NUMBER(a) == AS_NUMBER(b);
  case VAL_FLOAT:
    return AS_FLOAT(a) == AS_FLOAT(b);
  case VAL_OBJ:
    switch (OBJ_TYPE(a)) {
    case OBJ_STRING: {
      obj_string_t *a_str = AS_STRING(a);
      obj_string_t *b_str = AS_STRING(b);

      if (a_str->interned && b_str->interned)
        return a_str == b_str;
      if (a_str->length != b_str->length)
        return false;
      return memcmp(a_str->chars, b_str->chars, a_str->length) == 0;
    } break;
    case OBJ_VECTOR: {
      obj_vector_t *a_vec = AS_VECTOR(a);
      obj_vector_t *b_vec = AS_VECTOR(b);

      if (a_vec->count != b_vec->count)
        return false;

      for (int i = 0; i < a_vec->count; i++)
        if (!values_equal(a_vec->values[i], b_vec->values[i]))
          return false;

      return true;
    } break;
    case OBJ_LIST: {
      obj_list_t *a_list = AS_LIST(a);
      obj_list_t *b_list = AS_LIST(b);

      if (a_list->count != b_list->count)
        return false;

      for (int i = 0; i < a_list->count; i++)
        if (!values_equal(a_list->values[i], b_list->values[i]))
          return false;

      return true;
    } break;
    default:
      return AS_OBJ(a) == AS_OBJ(b);
    }
  case VAL_ANY:
    return false;
  }
}
