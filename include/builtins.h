#ifndef XYL_BUILTINS_H
#define XYL_BUILTINS_H

#include "value.h"

#define LIB "/lib/"
#define LIB_LEN 5
#define EXT ".xyl"
#define EXT_LEN 4

#define xyl_builtin(name) value_t builtin_##name(int argc, value_t *argv)

// IO
xyl_builtin(print);
xyl_builtin(println);
xyl_builtin(printf);
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

// Utils
xyl_builtin(typeof);
xyl_builtin(isinstance);
xyl_builtin(exit);
xyl_builtin(argv);

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

#endif
