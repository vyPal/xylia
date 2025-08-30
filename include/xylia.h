#ifndef XYL_XYLIA_H
#define XYL_XYLIA_H

typedef struct {
  const char *name;
} func_xyl_y;

typedef struct {
  func_xyl_y *functions;
  int capacity;
  int count;
} lib_xyl_t;

lib_xyl_t *new_lib(void);
void register_function(void);

#endif
