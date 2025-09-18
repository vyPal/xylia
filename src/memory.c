#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define GC_HEAP_GROW_FACTOR 2

char *read_file(const char *path) {
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    fprintf(stderr, "Could not open file '%s'\n", path);
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  size_t size = ftell(f);
  rewind(f);

  char *buffer = (char *)malloc(size + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read '%s'\n", path);
    fclose(f);
    return NULL;
  }

  size_t bytes_read = fread(buffer, sizeof(char), size, f);
  if (bytes_read < size) {
    fprintf(stderr, "Could not read file '%s'\n", path);
    free(buffer);
    fclose(f);
    return NULL;
  }

  buffer[bytes_read] = '\0';

  fclose(f);
  return buffer;
}

void *reallocate(void *ptr, size_t old_size, size_t new_size) {
  vm.bytes_allocated += new_size - old_size;
  if (new_size > old_size) {
#ifndef DEBUG_STRESS_GC
    if (vm.bytes_allocated > vm.next_gc)
#endif
      collect_garbage();
  }
  if (new_size == 0) {
    if (old_size != 0)
      free(ptr);
    return NULL;
  }

  void *result = realloc(ptr, new_size);
  if (result == NULL) {
    perror("realloc");
    exit(1);
  }

  return result;
}

void mark_object(obj_t *object) {
  if (object == NULL)
    return;
  if (object->is_marked)
    return;
  object->is_marked = true;

  if (vm.gray_capacity <= vm.gray_count) {
    vm.gray_capacity = GROW_CAPACITY(vm.gray_capacity);
    vm.gray_stack =
        (obj_t **)realloc(vm.gray_stack, sizeof(obj_t *) * vm.gray_capacity);
    if (!vm.gray_stack) {
      runtime_error(-1, "Failed to realloc gray_stack");
      vm.gray_capacity = 0;
      vm.gray_count = 0;
      vm.gray_stack = NULL;
      return;
    }
  }

  vm.gray_stack[vm.gray_count++] = object;
}

void mark_value(value_t value) {
  if (IS_OBJ(value))
    mark_object(AS_OBJ(value));
}

static void mark_array(value_array_t *array) {
  for (int i = 0; i < array->count; i++)
    mark_value(array->values[i]);
}

static void blacken_object(obj_t *object) {
  switch (object->type) {
  case OBJ_STRING:
  case OBJ_BUILTIN:
  case OBJ_FILE:
  case OBJ_ANY:
    break;
  case OBJ_VECTOR: {
    obj_vector_t *vector = (obj_vector_t *)object;
    for (int i = 0; i < vector->count; i++)
      mark_value(vector->values[i]);
  } break;
  case OBJ_LIST: {
    obj_list_t *list = (obj_list_t *)object;
    if (list->values != NULL)
      for (int i = 0; i < list->count; i++)
        mark_value(list->values[i]);
  } break;
  case OBJ_CLASS: {
    obj_class_t *clas = (obj_class_t *)object;
    mark_object((obj_t *)clas->name);
    mark_table(&clas->methods);
  } break;
  case OBJ_BOUND_METHOD: {
    obj_bound_method_t *bound = (obj_bound_method_t *)object;
    mark_value(bound->receiver);
    mark_object((obj_t *)bound->method);
  } break;
  case OBJ_INSTANCE: {
    obj_instance_t *instance = (obj_instance_t *)object;
    mark_object((obj_t *)instance->clas);
    mark_table(&instance->fields);
  } break;
  case OBJ_CLOSURE: {
    obj_closure_t *closure = (obj_closure_t *)object;
    mark_object((obj_t *)closure->function);
    for (int i = 0; i < closure->upvalue_count; i++)
      mark_object((obj_t *)closure->upvalues[i]);
  } break;
  case OBJ_FUNCTION: {
    obj_function_t *function = (obj_function_t *)object;
    mark_object((obj_t *)function->name);
    mark_array(&function->chunk.constants);
    if (function->globals != NULL)
      mark_table(function->globals);
  } break;
  case OBJ_UPVALUE:
    mark_value(((obj_upvalue_t *)object)->closed);
    break;
  case OBJ_MODULE: {
    obj_module_t *module = (obj_module_t *)object;
    mark_object((obj_t *)module->name);
    mark_object((obj_t *)module->init);
    mark_table(&module->globals);
  } break;
  case OBJ_RANGE: {
    obj_range_t *range = (obj_range_t *)object;
    mark_value(range->from);
    mark_value(range->to);
  } break;
  }
}

static void free_object(obj_t *object) {
  switch (object->type) {
  case OBJ_STRING: {
    obj_string_t *string = (obj_string_t *)object;
    FREE_ARRAY(char, string->chars, string->length + 1);
    FREE(obj_string_t, object);
  } break;
  case OBJ_VECTOR: {
    obj_vector_t *vector = (obj_vector_t *)object;
    FREE_ARRAY(value_t, vector->values, vector->capacity);
    FREE(obj_vector_t, object);
  } break;
  case OBJ_LIST: {
    obj_list_t *list = (obj_list_t *)object;
    if (list->count != 0)
      FREE_ARRAY(value_t, list->values, list->count);
    FREE(obj_list_t, object);
  } break;
  case OBJ_FILE: {
    obj_file_t *file = (obj_file_t *)object;
    if (file->open) {
      fclose(file->file);
      file->open = false;
      file->readable = false;
      file->writable = false;
    }
    FREE(obj_file_t, object);
  } break;
  case OBJ_CLASS: {
    obj_class_t *clas = (obj_class_t *)object;
    free_table(&clas->methods);
    FREE(obj_class_t, object);
  } break;
  case OBJ_BOUND_METHOD:
    FREE(obj_bound_method_t, object);
    break;
  case OBJ_INSTANCE: {
    obj_instance_t *instance = (obj_instance_t *)object;
    free_table(&instance->fields);
    FREE(obj_instance_t, object);
  } break;
  case OBJ_CLOSURE: {
    obj_closure_t *closure = (obj_closure_t *)object;
    FREE_ARRAY(obj_upvalue_t *, closure->upvalues, closure->upvalue_count);
    FREE(obj_closure_t, object);
  } break;
  case OBJ_FUNCTION: {
    obj_function_t *function = (obj_function_t *)object;
    free_chunk(&function->chunk);
    FREE(obj_function_t, object);
  } break;
  case OBJ_BUILTIN:
    FREE(obj_builtin_t, object);
    break;
  case OBJ_UPVALUE:
    FREE(obj_upvalue_t, object);
    break;
  case OBJ_MODULE: {
    obj_module_t *module = (obj_module_t *)object;
    free_table(&module->globals);
    FREE(obj_module_t, object);
  } break;
  case OBJ_RANGE:
    FREE(obj_range_t, object);
    break;
  case OBJ_ANY:
    break;
  }
}

void free_objects(void) {
  obj_t *object = vm.objects;
  while (object != NULL) {
    obj_t *next = object->next;
    free_object(object);
    object = next;
  }

  if (vm.gray_stack != NULL)
    free(vm.gray_stack);
}

static void mark_roots(void) {
  for (value_t *slot = vm.stack; slot < vm.stack_top; slot++)
    mark_value(*slot);

  for (int i = 0; i < vm.frame_count; i++) {
    mark_object((obj_t *)vm.frames[i].closure);
    mark_table(vm.frames[i].globals);
  }

  for (obj_upvalue_t *upvalue = vm.open_upvalues; upvalue != NULL;
       upvalue = upvalue->next)
    mark_object((obj_t *)upvalue);

  for (int i = 0; i < VM_STR_MAX; i++)
    mark_object((obj_t *)vm.vm_strings[i]);

  if (vm.args)
    mark_object((obj_t *)vm.args);

  mark_table(&vm.module_lookup);
  mark_table(&vm.builtins);
  mark_table(&vm.strings);

  mark_compiler_roots();
}

static void trace_references(void) {
  while (vm.gray_count > 0) {
    obj_t *obj = vm.gray_stack[--vm.gray_count];
    blacken_object(obj);
  }
}

static void sweep(void) {
  obj_t *previous = NULL;
  obj_t *object = vm.objects;
  while (object != NULL) {
    if (object->is_marked) {
      object->is_marked = false;
      previous = object;
      object = object->next;
    } else {
      obj_t *unreached = object;
      object = object->next;
      if (previous == NULL)
        vm.objects = object;
      else
        previous->next = object;

      free_object(unreached);
    }
  }
}

void collect_garbage(void) {
  mark_roots();
  trace_references();
  table_remove_white(&vm.strings);
  sweep();

  vm.next_gc = vm.bytes_allocated * GC_HEAP_GROW_FACTOR;
}
