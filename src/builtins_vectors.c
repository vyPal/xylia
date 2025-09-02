#include "builtins.h"
#include "memory.h"
#include "vm.h"

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
