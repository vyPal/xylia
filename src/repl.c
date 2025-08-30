#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "repl.h"
#include "vm.h"

extern bool failed;

static inline void print_prompt(void) {
  printf("\x1b[33m>>>\x1b[0m ");
}

void repl(void) {
  size_t capacity = 64;
  size_t count = 0;
  char *buffer = (char *)malloc(capacity);

  if (!buffer) {
    runtime_error("Failed to allocate memory for repl");
    failed = true;
    return;
  }

  int c;
  print_prompt();
  while ((c = fgetc(stdin)) != EOF) {
    if (count + 2 >= capacity) {
      capacity *= 2;
      char *new_buffer = realloc(buffer, capacity);
      if (!new_buffer) {
        runtime_error("Failed to reallocate memory for repl");
        failed = true;
        free(buffer);
        return;
      }
      buffer = new_buffer;
    }

    buffer[count++] = (char)c;
    buffer[count] = '\0';

    if (c == '\n') {
      if (count != 1) {
        if (count >= 2 && buffer[count - 2] != ';' && buffer[count - 2] != '}')
          buffer[count - 1] = ';';
        interpret(buffer);
        if (vm.signal != SIG_NONE)
          break;
      }
      print_prompt();
      buffer[0] = '\0';
      count = 0;
    }
  }

  free(buffer);
}
