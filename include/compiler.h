#ifndef XYL_COMPILER_H
#define XYL_COMPILER_H

#include "object.h"
#include "value.h"

typedef struct type_param type_param_t;

struct type_param {
  obj_string_t *type_name;
  type_param_t *next; // Linked list for multiple parameters
};

typedef struct {
  bool has_hint;
  obj_string_t *base_type;
  type_param_t *type_params;
  bool is_generic;
} type_hint_t;

obj_module_t *compile(const char *source, obj_string_t *path,
                      obj_string_t *file);
void mark_compiler_roots(void);

#endif
