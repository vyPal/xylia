#ifndef XYL_MEMORY_H
#define XYL_MEMORY_H

#include <stddef.h>

#include "value.h"

#define GROW_FACTOR 2

#define ALLOCATE(type, count)                                                  \
  (type *)reallocate(NULL, 0, sizeof(type) * (count))

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity) * GROW_FACTOR)

#define GROW_ARRAY(type, pointer, old_size, new_size)                          \
  (type *)reallocate(pointer, sizeof(type) * (old_size),                       \
                     sizeof(type) * (new_size))

#define FREE_ARRAY(type, pointer, old_size)                                    \
  reallocate(pointer, sizeof(type) * (old_size), 0)

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

char *read_file(const char *path);
void *reallocate(void *ptr, size_t old_size, size_t new_size);
void mark_object(obj_t *object);
void mark_value(value_t value);
void collect_garbage(void);
void free_objects(void);

#endif
