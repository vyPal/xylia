#ifndef XYL_COMPILER_H
#define XYL_COMPILER_H

#include "object.h"
#include "value.h"

obj_module_t *compile(const char *source, obj_string_t *path);
void mark_compiler_roots(void);

#endif
