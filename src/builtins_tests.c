#include "builtins.h"
#include "vm.h"

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
