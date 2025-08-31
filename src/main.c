#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "repl.h"
#include "vm.h"

#ifdef DEBUG
#include <execinfo.h>
#include <linux/limits.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

void segfault_handler(int sig) {
  void *array[20];
  size_t size = backtrace(array, 20);

  fprintf(stderr, "\n=== Caught signal %d (%s) ===\n", sig, strsignal(sig));
  fprintf(stderr, "Stack trace (addresses):\n");

  char exe_path[PATH_MAX];
  ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
  if (len == -1) {
    perror("readlink");
    exit(1);
  }
  exe_path[len] = '\0';

  for (size_t i = 0; i < size; i++) {
    fprintf(stderr, "[%zu] %p\n", i, array[i]);

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "addr2line -fpe %s %p", exe_path, array[i]);
    FILE *f = popen(cmd, "r");
    if (f) {
      char buf[1024];
      if (fgets(buf, sizeof(buf), f))
        fprintf(stderr, "    %s", buf);
      pclose(f);
    }
  }

  exit(1);
}
#endif

bool failed = false;

static void run_file(const char *path) {
  char *source = read_file(path);
  if (!source) {
    failed = true;
    return;
  }

  result_t result = interpret(source);
  free(source);

  if (result != RESULT_OK)
    failed = true;
}

int main(int argc, char **argv) {
#ifdef DEBUG
  struct sigaction sa;
  sa.sa_handler = (void (*)(int))segfault_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sigaction(SIGSEGV, &sa, NULL);
#endif

  init_vm();

  char *file = NULL;
  bool repl = false;

  if (argc == 1) {
    repl = true;
    set_args(0, argv + 1);
  } else {
    file = argv[1];
    set_args(argc - 1, argv + 1);
  }

  if (!failed) {
    if (repl)
      run_repl();
    else if (file)
      run_file(file);
    else {
      fprintf(stderr, "Usage [path [flags]]\n");
      failed = true;
    }
  }

  int exit_code = failed ? 1 : vm.exit_code;
  switch (vm.signal) {
  case SIG_STACK_OVERFLOW:
    printf("Stack overflow detected\n");
    break;
  case SIG_STACK_UNDERFLOW:
    printf("Stack underflow detected\n");
    break;
  default:
    break;
  }

  free_vm();

  return exit_code;
}
