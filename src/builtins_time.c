#include <time.h>
#include <unistd.h>

#include "builtins.h"
#include "vm.h"

xyl_builtin(now) {
  xyl_builtin_signature(now, 0, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  double now = (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
  return FLOAT_VAL(now);
}

xyl_builtin(clock) {
  xyl_builtin_signature(clock, 0, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  return FLOAT_VAL((double)clock() / CLOCKS_PER_SEC);
}

xyl_builtin(sleep) {
  xyl_builtin_signature(sleep, 1, ARGC_EXACT, {VAL_ANY, OBJ_ANY});
  if (IS_NUMBER(argv[0])) {
    sleep(AS_NUMBER(argv[0]));
    return NIL_VAL;
  } else if (IS_FLOAT(argv[0])) {
    double sleep_time = AS_FLOAT(argv[0]);
    if (sleep_time <= 0.0)
      return NIL_VAL;

    struct timespec req, rem;
    req.tv_sec = (time_t)sleep_time;
    req.tv_nsec = (long)((sleep_time - (double)req.tv_sec) * 1000000000.0);

    if (req.tv_nsec >= 1000000000) {
      req.tv_sec += req.tv_nsec / 1000000000;
      req.tv_nsec %= 1000000000;
    }

    while (nanosleep(&req, &rem) == -1)
      req = rem;
    return NIL_VAL;
  }

  runtime_error(
      "Expected argument 1 in 'sleep' to be 'number' or 'float' but got '%s'",
      value_type_to_str(argv[0].type));
  return NIL_VAL;
}
