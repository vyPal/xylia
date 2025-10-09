#include "builtins.h"
#include "object.h"
#include "value.h"
#include "vm.h"

// ok(value) -> Result
// Creates a new Ok result containing the given value
xyl_builtin(ok) {
  xyl_builtin_signature(ok, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  return OBJ_VAL(new_result_ok(argv[0]));
}

// err(error) -> Result
// Creates a new Err result containing the given error value
xyl_builtin(err) {
  xyl_builtin_signature(err, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  return OBJ_VAL(new_result_err(argv[0]));
}

// is_ok(result) -> bool
// Returns true if the result is Ok, false otherwise
xyl_builtin(is_ok) {
  xyl_builtin_signature(is_ok, 1, ARGC_EXACT, {VAL_OBJ, OBJ_RESULT});
  obj_result_t *result = AS_RESULT(argv[0]);
  return BOOL_VAL(result->is_ok);
}

// is_err(result) -> bool
// Returns true if the result is Err, false otherwise
xyl_builtin(is_err) {
  xyl_builtin_signature(is_err, 1, ARGC_EXACT, {VAL_OBJ, OBJ_RESULT});
  obj_result_t *result = AS_RESULT(argv[0]);
  return BOOL_VAL(!result->is_ok);
}

// unwrap(result) -> any
// Returns the value inside an Ok result, or causes a runtime error for Err
xyl_builtin(unwrap) {
  xyl_builtin_signature(unwrap, 1, ARGC_EXACT, {VAL_OBJ, OBJ_RESULT});
  obj_result_t *result = AS_RESULT(argv[0]);

  if (result->is_ok) {
    return result->value;
  } else {
    runtime_error(-1, "Called unwrap on an Err result");
    return NIL_VAL;
  }
}

// unwrap_or(result, default) -> any
// Returns the value inside an Ok result, or the default value for Err
xyl_builtin(unwrap_or) {
  xyl_builtin_signature(unwrap_or, 2, ARGC_EXACT, {VAL_OBJ, OBJ_RESULT}, {VAL_ANY, OBJ_ANY});
  obj_result_t *result = AS_RESULT(argv[0]);

  if (result->is_ok) {
    return result->value;
  } else {
    return argv[1];
  }
}

// unwrap_err(result) -> any
// Returns the error value inside an Err result, or causes a runtime error for Ok
xyl_builtin(unwrap_err) {
  xyl_builtin_signature(unwrap_err, 1, ARGC_EXACT, {VAL_OBJ, OBJ_RESULT});
  obj_result_t *result = AS_RESULT(argv[0]);

  if (!result->is_ok) {
    return result->value;
  } else {
    runtime_error(-1, "Called unwrap_err on an Ok result");
    return NIL_VAL;
  }
}

// expect(result, message) -> any
// Returns the value inside an Ok result, or causes a runtime error with custom message for Err
xyl_builtin(expect) {
  xyl_builtin_signature(expect, 2, ARGC_EXACT, {VAL_OBJ, OBJ_RESULT}, {VAL_OBJ, OBJ_STRING});
  obj_result_t *result = AS_RESULT(argv[0]);
  obj_string_t *message = AS_STRING(argv[1]);

  if (result->is_ok) {
    return result->value;
  } else {
    runtime_error(-1, "%s", message->chars);
    return NIL_VAL;
  }
}
