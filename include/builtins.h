#ifndef XYL_BUILTINS_H
#define XYL_BUILTINS_H

#include "object.h"
#include "value.h"

#define LIB "/lib/"
#define LIB_LEN 5
#define EXT ".xyl"
#define EXT_LEN 4

#define xyl_builtin(name) value_t builtin_##name(int argc, value_t *argv)

#define xyl_builtin_signature(name, arg_count, comparison, ...)                \
  const builtin_arg_t name##_args[] = {__VA_ARGS__};                           \
  const builtin_signature_t name##_signature = {#name, arg_count,              \
                                                (builtin_arg_t *)name##_args}; \
  if (!check_signature(&name##_signature, argc, argv, comparison))             \
  return NIL_VAL

#define xyl_check_signature(name, comparison)                                  \
  check_signature(&name##_signature, argc, argv, comparison)

typedef struct {
  char *data;
  size_t capacity;
  size_t length;
} string_builder_t;

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

void sb_init(string_builder_t *sb);
void sb_append(string_builder_t *sb, const char *str, size_t len);
void sb_free(string_builder_t *sb);
obj_string_t *value_to_string(value_t value, bool literal);

const char *value_type_to_str(value_type_t type);
const char *obj_type_to_str(obj_type_t type);
bool check_signature(const builtin_signature_t *signature, int argc,
                     value_t *argv, argc_comparison_t comparison);

// IO
xyl_builtin(print);
xyl_builtin(println);
xyl_builtin(printf);

xyl_builtin(fprint);
xyl_builtin(fprintln);
xyl_builtin(fprintf);

xyl_builtin(stdin);
xyl_builtin(stdout);
xyl_builtin(stderr);

xyl_builtin(input);

xyl_builtin(open);
xyl_builtin(close);
xyl_builtin(read);
xyl_builtin(write);

// Vectors
xyl_builtin(len);
xyl_builtin(append);
xyl_builtin(pop);
xyl_builtin(insert);
xyl_builtin(remove);
xyl_builtin(slice);

// Array
xyl_builtin(array);
xyl_builtin(resize);

// Utils
xyl_builtin(typeof);
xyl_builtin(isinstance);
xyl_builtin(hasmethod);
xyl_builtin(getclass);
xyl_builtin(exit);
xyl_builtin(argv);
xyl_builtin(hash);

xyl_builtin(import);

// Casts
xyl_builtin(string);
xyl_builtin(number);
xyl_builtin(float);
xyl_builtin(bool);
xyl_builtin(vector);
xyl_builtin(list);

// Tests
xyl_builtin(case_failed);

xyl_builtin(assert_true);
xyl_builtin(assert_false);
xyl_builtin(assert_eq);
xyl_builtin(assert_neq);

// Math
xyl_builtin(abs);
xyl_builtin(min);
xyl_builtin(max);

xyl_builtin(sin);
xyl_builtin(cos);
xyl_builtin(tan);
xyl_builtin(asin);
xyl_builtin(acos);
xyl_builtin(atan);
xyl_builtin(atan2);

xyl_builtin(sqrt);
xyl_builtin(pow);
xyl_builtin(log);
xyl_builtin(exp);

xyl_builtin(floor);
xyl_builtin(ceil);
xyl_builtin(round);

xyl_builtin(random);
xyl_builtin(randomseed);

// Time
xyl_builtin(now);
xyl_builtin(clock);
xyl_builtin(sleep);
xyl_builtin(localtime);

#endif
