#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "random.h"
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

// CLI state
static bool failed = false;
static bool verbose = false;

// Version information
#define XYLIA_VERSION_MAJOR 0
#define XYLIA_VERSION_MINOR 0
#define XYLIA_VERSION_PATCH 1
#define XYLIA_VERSION_STRING "0.0.1"

// Forward declarations
static void show_version(void);
static void show_help(const char *program_name);
static void run_file(const char *path);
static void run_repl_command(void);
static void run_command(int argc, char **argv);
static bool parse_global_args(int *argc, char ***argv);

// Print version information
static void show_version(void) {
  printf("Xylia %s\n", XYLIA_VERSION_STRING);
  printf("A hobby programming language written in c\n");
}

// Print help information
static void show_help(const char *program_name) {
  printf("Xylia %s - A hobby programming language written in c\n\n", XYLIA_VERSION_STRING);

  printf("USAGE:\n");
  printf("    %s [OPTIONS] [SUBCOMMAND] [ARGS...]\n", program_name);
  printf("    %s [OPTIONS] <file> [ARGS...]     Run a Xylia script (default)\n", program_name);
  printf("\n");

  printf("OPTIONS:\n");
  printf("    -h, --help       Show this help message\n");
  printf("    -V, --version    Show version information\n");
  printf("    -v, --verbose    Enable verbose output\n");
  printf("\n");

  printf("SUBCOMMANDS:\n");
  printf("    run <file>       Run a Xylia script file\n");
  printf("    repl             Start interactive REPL session\n");
  printf("    help             Show this help message\n");
  printf("    version          Show version information\n");
  printf("\n");

  printf("ARGUMENTS:\n");
  printf("    <file>           Path to Xylia script file to execute\n");
  printf("    [ARGS...]        Arguments passed to the script\n");
  printf("\n");

  printf("EXAMPLES:\n");
  printf("    %s hello.xyl                    # Run hello.xyl\n", program_name);
  printf("    %s run hello.xyl arg1 arg2      # Run hello.xyl with arguments\n", program_name);
  printf("    %s repl                         # Start interactive session\n", program_name);
  printf("    %s --version                    # Show version\n", program_name);
  printf("    %s --help                       # Show this help\n", program_name);
  printf("\n");

  printf("ENVIRONMENT:\n");
  printf("    XYL_HOME         Path to Xylia standard library (required)\n");
  printf("\n");

  printf("For more information, visit: https://github.com/your-org/xylia\n");
}

// Run a script file
static void run_file(const char *path) {
  if (verbose) {
    printf("Running file: %s\n", path);
  }

  char *source = read_file(path);
  if (!source) {
    failed = true;
    return;
  }

  result_t result = interpret(source, path);
  free(source);

  if (result != RESULT_OK) {
    failed = true;
  }
}

// Run REPL command
static void run_repl_command(void) {
  if (verbose) {
    printf("Starting Xylia REPL...\n");
  }
  set_args(0, NULL);
  run_repl();
}

// Run subcommand
static void run_command(int argc, char **argv) {
  if (argc < 1) {
    fprintf(stderr, "Error: No subcommand provided\n");
    failed = true;
    return;
  }

  const char *cmd = argv[0];

  if (strcmp(cmd, "run") == 0) {
    if (argc < 2) {
      fprintf(stderr, "Error: 'run' command requires a file argument\n");
      fprintf(stderr, "Usage: run <file> [args...]\n");
      failed = true;
      return;
    }

    const char *file = argv[1];
    set_args(argc - 2, argv + 2);
    run_file(file);

  } else if (strcmp(cmd, "repl") == 0) {
    if (argc > 1) {
      fprintf(stderr, "Warning: 'repl' command ignores additional arguments\n");
    }
    run_repl_command();

  } else if (strcmp(cmd, "help") == 0) {
    show_help("xylia");

  } else if (strcmp(cmd, "version") == 0) {
    show_version();

  } else {
    fprintf(stderr, "Error: Unknown subcommand '%s'\n", cmd);
    fprintf(stderr, "Run 'xylia help' for usage information\n");
    failed = true;
  }
}

// Parse global arguments (returns true if execution should continue)
static bool parse_global_args(int *argc, char ***argv) {
  int new_argc = 0;
  char **new_argv = *argv;

  for (int i = 0; i < *argc; i++) {
    const char *arg = (*argv)[i];

    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
      show_help("xylia");
      return false;
    } else if (strcmp(arg, "-V") == 0 || strcmp(arg, "--version") == 0) {
      show_version();
      return false;
    } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
      verbose = true;
      continue; // Don't add to new_argv
    } else if (arg[0] == '-') {
      // Unknown flag
      fprintf(stderr, "Error: Unknown option '%s'\n", arg);
      fprintf(stderr, "Run 'xylia --help' for usage information\n");
      failed = true;
      return false;
    } else {
      // Keep this argument (not a flag)
      new_argv[new_argc++] = (*argv)[i];
    }
  }

  *argc = new_argc;
  return true;
}

// Check if string looks like a file path (contains .xyl extension or path separators)
static bool looks_like_file(const char *str) {
  if (!str) return false;

  // Check for .xyl extension
  size_t len = strlen(str);
  if (len > 4 && strcmp(str + len - 4, ".xyl") == 0) {
    return true;
  }

  // Check for path separators
  if (strchr(str, '/') != NULL || strchr(str, '\\') != NULL) {
    return true;
  }

  // Check if it's not a known subcommand
  if (strcmp(str, "run") == 0 || strcmp(str, "repl") == 0 ||
      strcmp(str, "help") == 0 || strcmp(str, "version") == 0) {
    return false;
  }

  return true;
}

// Check if file exists and is readable
static bool file_exists(const char *path) {
  FILE *f = fopen(path, "r");
  if (f) {
    fclose(f);
    return true;
  }
  return false;
}

int main(int argc, char **argv) {
#ifdef DEBUG
  struct sigaction sa;
  sa.sa_handler = (void (*)(int))segfault_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sigaction(SIGSEGV, &sa, NULL);
#endif

  // Skip program name
  argc--;
  argv++;

  // Parse global arguments
  if (!parse_global_args(&argc, &argv)) {
    return 0; // Help or version was shown
  }

  // Initialize VM and random seed
  uint64_t seed = get_seed();
  mt_seed_u64(seed);
  init_vm();

  // Determine what to do based on arguments
  if (argc == 0) {
    // No arguments - start REPL
    run_repl_command();

  } else if (argc == 1 && looks_like_file(argv[0])) {
    // Single argument that looks like a file - run it
    const char *file = argv[0];

    if (!file_exists(file)) {
      fprintf(stderr, "Error: File '%s' not found\n", file);
      failed = true;
    } else {
      set_args(0, NULL);
      run_file(file);
    }

  } else if (looks_like_file(argv[0])) {
    // First argument looks like a file, rest are script arguments
    const char *file = argv[0];

    if (!file_exists(file)) {
      fprintf(stderr, "Error: File '%s' not found\n", file);
      failed = true;
    } else {
      set_args(argc - 1, argv + 1);
      run_file(file);
    }

  } else {
    // First argument doesn't look like a file - treat as subcommand
    run_command(argc, argv);
  }

  // Handle exit codes and cleanup
  int exit_code = failed ? 1 : vm.exit_code;

  switch (vm.signal) {
  case SIG_STACK_OVERFLOW:
    fprintf(stderr, "Error: Stack overflow detected\n");
    exit_code = 2;
    break;
  case SIG_STACK_UNDERFLOW:
    fprintf(stderr, "Error: Stack underflow detected\n");
    exit_code = 2;
    break;
  default:
    break;
  }

  free_vm();
  return exit_code;
}
