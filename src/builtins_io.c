#include <stdio.h>
#include <stdlib.h>

#include "builtins.h"
#include "object.h"
#include "value.h"
#include "vm.h"

xyl_builtin(print) {
  xyl_builtin_signature(print, 1, ARGC_EXACT, {VAL_OBJ, OBJ_LIST});
  obj_list_t *args = AS_LIST(argv[0]);

  for (int i = 0; i < args->count; i++) {
    if (i != 0)
      putchar(' ');
    print_value(args->values[i], false);
  }

  return NIL_VAL;
}

xyl_builtin(println) {
  xyl_builtin_signature(println, 1, ARGC_EXACT, {VAL_OBJ, OBJ_LIST});
  obj_list_t *args = AS_LIST(argv[0]);

  for (int i = 0; i < args->count; i++) {
    if (i != 0)
      putchar(' ');
    print_value(args->values[i], false);
  }
  putchar('\n');

  return NIL_VAL;
}

xyl_builtin(printf) {
  xyl_builtin_signature(printf, 1, ARGC_EXACT, {VAL_OBJ, OBJ_LIST});
  obj_list_t *args = AS_LIST(argv[0]);

  if (args->count == 0 || !IS_STRING(args->values[0])) {
    runtime_error(-1, "Expected at least 1 argument in 'printf' but got %d",
                  args->count);
    return NIL_VAL;
  }

  obj_string_t *fmt = AS_STRING(args->values[0]);
  int fmt_count = 1;

  for (int i = 0; i < fmt->length; i++) {
    if (fmt->chars[i] == '\\' && i + 1 < fmt->length &&
        fmt->chars[i + 1] == '{') {
      putchar('{');
      i++;
    } else if (fmt->chars[i] == '{' && i + 1 < fmt->length &&
               fmt->chars[i + 1] == '}') {
      if (fmt_count >= args->count) {
        runtime_error(-1, "Not enough arguments in printf");
        return NIL_VAL;
      }
      print_value(args->values[fmt_count++], false);
      i++;
    } else
      putchar(fmt->chars[i]);
  }

  return NIL_VAL;
}

xyl_builtin(input) {
  xyl_builtin_signature(input, 1, ARGC_LESS_OR_EXACT, {VAL_OBJ, OBJ_STRING});

  if (argc == 1)
    print_value(argv[0], false);

  size_t capacity = 64;
  size_t length = 0;
  char *buffer = (char *)malloc(capacity);

  if (!buffer) {
    runtime_error(-1, "[%s:%s] Failed to allocate memory for input buffer",
                  __FILE_NAME__, __PRETTY_FUNCTION__);
    return NIL_VAL;
  }

  int c;
  while ((c = fgetc(stdin)) != EOF) {
    if (length + 1 >= capacity) {
      capacity *= 2;
      char *new_buffer = realloc(buffer, capacity);
      if (!new_buffer) {
        runtime_error(-1,
                      "[%s:%s] Failed to reallocate memory for input buffer",
                      __FILE_NAME__, __PRETTY_FUNCTION__);
        free(buffer);
        return NIL_VAL;
      }
      buffer = new_buffer;
    }

    if (c == '\n')
      break;

    buffer[length++] = (char)c;
  }

  if (length == 0 && c == EOF) {
    free(buffer);
    return NIL_VAL;
  }

  buffer[length] = '\0';
  obj_string_t *str = copy_string(buffer, length, false);

  return OBJ_VAL(str);
}

xyl_builtin(open) {
  xyl_builtin_signature(open, 2, ARGC_EXACT, {VAL_OBJ, OBJ_STRING},
                        {VAL_OBJ, OBJ_STRING});

  const char *path = AS_CSTRING(argv[0]);
  const char *mode = AS_CSTRING(argv[1]);
  obj_file_t *file = new_file(path, mode);

  return OBJ_VAL(file);
}

xyl_builtin(close) {
  xyl_builtin_signature(close, 1, ARGC_EXACT, {VAL_OBJ, OBJ_FILE});

  obj_file_t *file = AS_FILE(argv[0]);
  if (!file->open)
    return NIL_VAL;

  fclose(file->file);
  file->open = false;
  file->readable = false;
  file->writable = false;

  return NIL_VAL;
}

xyl_builtin(read) {
  xyl_builtin_signature(read, 1, ARGC_EXACT, {VAL_OBJ, OBJ_FILE});

  obj_file_t *file = AS_FILE(argv[0]);
  if (!file->open) {
    runtime_error(-1, "File is closed");
    return NIL_VAL;
  }

  if (!file->readable) {
    runtime_error(-1, "File is not readable");
    return NIL_VAL;
  }

  long start_pos = ftell(file->file);
  if (start_pos < 0) {
    runtime_error(-1, "Invalid file position");
    return NIL_VAL;
  }

  if (fseek(file->file, 0, SEEK_END) != 0) {
    runtime_error(-1, "Failed to determine the end of file");
    return NIL_VAL;
  }

  long file_size = ftell(file->file);
  if (file_size < 0) {
    runtime_error(-1, "Invalid file size");
    return NIL_VAL;
  }

  char *buffer = (char *)malloc(file_size + 1);
  if (!buffer) {
    runtime_error(-1, "[%s:%s] Failed to allocate memory for input buffer",
                  __FILE_NAME__, __PRETTY_FUNCTION__);
    return NIL_VAL;
  }

  if (fseek(file->file, 0, SEEK_SET) != 0) {
    runtime_error(-1, "Failed to read the file");
    free(buffer);
    return NIL_VAL;
  }

  size_t read_size = fread(buffer, 1, file_size, file->file);
  buffer[read_size] = '\0';

  fseek(file->file, start_pos, SEEK_SET);

  return OBJ_VAL(copy_string(buffer, read_size, false));
}

xyl_builtin(write) {
  xyl_builtin_signature(write, 2, ARGC_EXACT, {VAL_OBJ, OBJ_FILE},
                        {VAL_OBJ, OBJ_STRING});

  obj_file_t *file = AS_FILE(argv[0]);
  const char *data = AS_CSTRING(argv[1]);
  if (!file->open) {
    runtime_error(-1, "File is closed");
    return NIL_VAL;
  }

  if (!file->writable) {
    runtime_error(-1, "File is not writable");
    return NIL_VAL;
  }

  if (fputs(data, file->file) == EOF) {
    runtime_error(-1, "Failed to write to file");
    return NIL_VAL;
  }
  fflush(file->file);

  return NIL_VAL;
}
