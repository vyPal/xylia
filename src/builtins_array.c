#include <stdint.h>

#include "builtins.h"
#include "memory.h"
#include "object.h"
#include "value.h"

xyl_builtin(array) {
  xyl_builtin_signature(array, 1, ARGC_EXACT, {VAL_NUMBER, OBJ_ANY});
  obj_array_t *array = new_array(AS_NUMBER(argv[0]));
  return OBJ_VAL(array);
}

xyl_builtin(resize) {
  xyl_builtin_signature(resize, 2, ARGC_EXACT, {VAL_OBJ, OBJ_ARRAY},
                        {VAL_NUMBER, OBJ_ANY});
  obj_array_t *array = AS_ARRAY(argv[0]);
  int64_t new_size = AS_NUMBER(argv[1]);

  int old_count = array->count;
  array->count = GROW_CAPACITY(old_count);
  array->values = GROW_ARRAY(value_t, array->values, old_count, array->count);

  for (int i = old_count; i < array->count; i++)
    array->values[i] = NIL_VAL;

  return OBJ_VAL(array);
}
