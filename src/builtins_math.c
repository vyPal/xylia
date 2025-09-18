#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "builtins.h"
#include "object.h"
#include "random.h"
#include "value.h"
#include "vm.h"

xyl_builtin(abs) {
  xyl_builtin_signature(abs, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  if (IS_NUMBER(argv[0]))
    return NUMBER_VAL(labs(AS_NUMBER(argv[0])));
  else if (IS_FLOAT(argv[0]))
    return FLOAT_VAL(fabs(AS_FLOAT(argv[0])));

  runtime_error(
      -1, "Expected argument 1 in 'abs' to be 'number' or 'float' but got '%s'",
      value_type_to_str(argv[0].type));
  return NIL_VAL;
}

xyl_builtin(min) {
  xyl_builtin_signature(min, 1, ARGC_EXACT, {VAL_OBJ, OBJ_LIST});
  obj_list_t *list = AS_LIST(argv[0]);
  value_t min = NIL_VAL;
  for (int i = 0; i < list->count; i++) {
    if (!IS_NUMBER(list->values[i]) && !IS_FLOAT(list->values[i])) {
      runtime_error(-1,
                    "Expected argument %d in 'min' to be 'number' or 'float' "
                    "but got '%s'",
                    i + 1, value_type_to_str(list->values[i].type));
      return NIL_VAL;
    }

    if (min.type == VAL_NIL)
      min = list->values[i];

    double num = IS_NUMBER(list->values[i]) ? (double)AS_NUMBER(list->values[i])
                                            : AS_FLOAT(list->values[i]);
    double min_num = IS_NUMBER(min) ? (double)AS_NUMBER(min) : AS_FLOAT(min);
    if (num < min_num)
      min = list->values[i];
  }

  return min;
}

xyl_builtin(max) {
  xyl_builtin_signature(max, 1, ARGC_EXACT, {VAL_OBJ, OBJ_LIST});
  obj_list_t *list = AS_LIST(argv[0]);
  value_t max = NIL_VAL;
  for (int i = 0; i < list->count; i++) {
    if (!IS_NUMBER(list->values[i]) && !IS_FLOAT(list->values[i])) {
      runtime_error(-1,
                    "Expected argument %d in 'max' to be 'number' or 'float' "
                    "but got '%s'",
                    i + 1, value_type_to_str(list->values[i].type));
      return NIL_VAL;
    }

    if (max.type == VAL_NIL)
      max = list->values[i];

    double num = IS_NUMBER(list->values[i]) ? (double)AS_NUMBER(list->values[i])
                                            : AS_FLOAT(list->values[i]);
    double max_num = IS_NUMBER(max) ? (double)AS_NUMBER(max) : AS_FLOAT(max);
    if (num > max_num)
      max = list->values[i];
  }

  return max;
}

xyl_builtin(sin) {
  xyl_builtin_signature(sin, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  if (IS_NUMBER(argv[0]))
    return NUMBER_VAL(sin((double)AS_NUMBER(argv[0])));
  else if (IS_FLOAT(argv[0]))
    return FLOAT_VAL(sin(AS_FLOAT(argv[0])));

  runtime_error(
      -1, "Expected argument 1 in 'sin' to be 'number' or 'float' but got '%s'",
      value_type_to_str(argv[0].type));
  return NIL_VAL;
}

xyl_builtin(cos) {
  xyl_builtin_signature(cos, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  if (IS_NUMBER(argv[0]))
    return NUMBER_VAL(cos((double)AS_NUMBER(argv[0])));
  else if (IS_FLOAT(argv[0]))
    return FLOAT_VAL(cos(AS_FLOAT(argv[0])));

  runtime_error(
      -1, "Expected argument 1 in 'cos' to be 'number' or 'float' but got '%s'",
      value_type_to_str(argv[0].type));
  return NIL_VAL;
}

xyl_builtin(tan) {
  xyl_builtin_signature(tan, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  if (IS_NUMBER(argv[0]))
    return NUMBER_VAL(tan((double)AS_NUMBER(argv[0])));
  else if (IS_FLOAT(argv[0]))
    return FLOAT_VAL(tan(AS_FLOAT(argv[0])));

  runtime_error(
      -1, "Expected argument 1 in 'tan' to be 'number' or 'float' but got '%s'",
      value_type_to_str(argv[0].type));
  return NIL_VAL;
}

xyl_builtin(asin) {
  xyl_builtin_signature(asin, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  if (IS_NUMBER(argv[0]))
    return NUMBER_VAL(asin((double)AS_NUMBER(argv[0])));
  else if (IS_FLOAT(argv[0]))
    return FLOAT_VAL(asin(AS_FLOAT(argv[0])));

  runtime_error(
      -1,
      "Expected argument 1 in 'asin' to be 'number' or 'float' but got '%s'",
      value_type_to_str(argv[0].type));
  return NIL_VAL;
}

xyl_builtin(acos) {
  xyl_builtin_signature(acos, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  if (IS_NUMBER(argv[0]))
    return NUMBER_VAL(acos((double)AS_NUMBER(argv[0])));
  else if (IS_FLOAT(argv[0]))
    return FLOAT_VAL(acos(AS_FLOAT(argv[0])));

  runtime_error(
      -1,
      "Expected argument 1 in 'acos' to be 'number' or 'float' but got '%s'",
      value_type_to_str(argv[0].type));
  return NIL_VAL;
}

xyl_builtin(atan) {
  xyl_builtin_signature(atan, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  if (IS_NUMBER(argv[0]))
    return NUMBER_VAL(atan((double)AS_NUMBER(argv[0])));
  else if (IS_FLOAT(argv[0]))
    return FLOAT_VAL(atan(AS_FLOAT(argv[0])));

  runtime_error(
      -1,
      "Expected argument 1 in 'atan' to be 'number' or 'float' but got '%s'",
      value_type_to_str(argv[0].type));
  return NIL_VAL;
}

xyl_builtin(atan2) {
  xyl_builtin_signature(atan2, 2, ARGC_EXACT, {VAL_FLOAT, OBJ_ANY},
                        {VAL_FLOAT, OBJ_ANY});
  return FLOAT_VAL(atan2(AS_FLOAT(argv[0]), AS_FLOAT(argv[1])));
}

xyl_builtin(sqrt) {
  xyl_builtin_signature(sqrt, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  if (IS_NUMBER(argv[0]))
    return NUMBER_VAL(sqrt((double)AS_NUMBER(argv[0])));
  else if (IS_FLOAT(argv[0]))
    return FLOAT_VAL(sqrt(AS_FLOAT(argv[0])));

  runtime_error(
      -1,
      "Expected argument 1 in 'sqrt' to be 'number' or 'float' but got '%s'",
      value_type_to_str(argv[0].type));
  return NIL_VAL;
}

xyl_builtin(pow) {
  xyl_builtin_signature(pow, 2, ARGC_EXACT, {VAL_ANY, OBJ_ANY},
                        {VAL_ANY, OBJ_ANY});
  if (!IS_NUMBER(argv[0]) && !IS_FLOAT(argv[0])) {
    runtime_error(
        -1,
        "Expected argument 1 in 'pow' to be 'number' or 'float' but got '%s'",
        value_type_to_str(argv[0].type));
    return NIL_VAL;
  }

  if (!IS_NUMBER(argv[1]) && !IS_FLOAT(argv[1])) {
    runtime_error(
        -1,
        "Expected argument 2 in 'pow' to be 'number' or 'float' but got '%s'",
        value_type_to_str(argv[1].type));
    return NIL_VAL;
  }

  double a = IS_FLOAT(argv[0]) ? AS_FLOAT(argv[0]) : (double)AS_NUMBER(argv[0]);
  double b = IS_FLOAT(argv[1]) ? AS_FLOAT(argv[1]) : (double)AS_NUMBER(argv[1]);

  return FLOAT_VAL(pow(a, b));
}

xyl_builtin(log) {
  xyl_builtin_signature(log, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  if (IS_NUMBER(argv[0]))
    return NUMBER_VAL(log((double)AS_NUMBER(argv[0])));
  else if (IS_FLOAT(argv[0]))
    return FLOAT_VAL(log(AS_FLOAT(argv[0])));

  runtime_error(
      -1, "Expected argument 1 in 'log' to be 'number' or 'float' but got '%s'",
      value_type_to_str(argv[0].type));
  return NIL_VAL;
}

xyl_builtin(exp) {
  xyl_builtin_signature(exp, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  if (IS_NUMBER(argv[0]))
    return NUMBER_VAL(exp((double)AS_NUMBER(argv[0])));
  else if (IS_FLOAT(argv[0]))
    return FLOAT_VAL(exp(AS_FLOAT(argv[0])));

  runtime_error(
      -1, "Expected argument 1 in 'exp' to be 'number' or 'float' but got '%s'",
      value_type_to_str(argv[0].type));
  return NIL_VAL;
}

xyl_builtin(floor) {
  xyl_builtin_signature(floor, 1, ARGC_EXACT, {VAL_FLOAT, OBJ_ANY});
  return NUMBER_VAL((int64_t)floor(AS_FLOAT(argv[0])));
}

xyl_builtin(ceil) {
  xyl_builtin_signature(ceil, 1, ARGC_EXACT, {VAL_FLOAT, OBJ_ANY});
  return NUMBER_VAL((int64_t)ceil(AS_FLOAT(argv[0])));
}

xyl_builtin(round) {
  xyl_builtin_signature(round, 1, ARGC_EXACT, {VAL_FLOAT, OBJ_ANY});
  return NUMBER_VAL((int64_t)round(AS_FLOAT(argv[0])));
}

xyl_builtin(random) {
  xyl_builtin_signature(random, 0, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  return FLOAT_VAL(mt_rand_double01());
}

xyl_builtin(randomseed) {
  xyl_builtin_signature(randomseed, 1, ARGC_EXACT, {VAL_NUMBER, OBJ_ANY});
  mt_seed_u64(AS_NUMBER(argv[0]));
  return NIL_VAL;
}
