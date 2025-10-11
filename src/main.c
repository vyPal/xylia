#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "memory.h"
#include "random.h"
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

// Subcommand dispatch table
typedef struct {
  const char *name;
  subcommand_fn_t func;
  const char *description;
} subcommand_t;

static const subcommand_t subcommands[] = {
  {"run", cli_run, "Run a Xylia script file"},
  {"repl", cli_repl, "Start interactive REPL session"},
  {NULL, NULL, NULL} // Sentinel
};

// Find subcommand by name
static const subcommand_t *find_subcommand(const char *name) {
  for (const subcommand_t *cmd = subcommands; cmd->name != NULL; cmd++) {
    if (strcmp(cmd->name, name) == 0) {
      return cmd;
    }
  }
  return NULL;
}

// Run subcommand dispatcher
static cli_result_t run_subcommand(int argc, char **argv, cli_context_t *ctx) {
  if (argc < 1) {
    fprintf(stderr, "Error: No subcommand provided\n");
    return CLI_INVALID_ARGS;
  }
  
  const char *cmd_name = argv[0];
  
  // Handle built-in subcommands
  if (strcmp(cmd_name, "help") == 0) {
    cli_show_help("xylia");
    return CLI_SUCCESS;
  } else if (strcmp(cmd_name, "version") == 0) {
    cli_show_version();
    return CLI_SUCCESS;
  }
  
  // Find and execute subcommand
  const subcommand_t *cmd = find_subcommand(cmd_name);
  if (cmd == NULL) {
    fprintf(stderr, "Error: Unknown subcommand '%s'\n", cmd_name);
    fprintf(stderr, "Run 'xylia help' for usage information\n");
    return CLI_ERROR;
  }
  
  return cmd->func(argc - 1, argv + 1, ctx);
}

// Parse global arguments (returns true if execution should continue)
static bool parse_global_args(int *argc, char ***argv, cli_context_t *ctx) {
  int new_argc = 0;
  char **new_argv = *argv;
  
  for (int i = 0; i < *argc; i++) {
    const char *arg = (*argv)[i];
    
    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
      cli_show_help("xylia");
      return false;
    } else if (strcmp(arg, "-V") == 0 || strcmp(arg, "--version") == 0) {
      cli_show_version();
      return false;
    } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
      ctx->verbose = true;
      continue; // Don't add to new_argv
    } else if (arg[0] == '-') {
      // Unknown flag
      fprintf(stderr, "Error: Unknown option '%s'\n", arg);
      fprintf(stderr, "Run 'xylia --help' for usage information\n");
      ctx->failed = true;
      return false;
    } else {
      // Keep this argument (not a flag)
      new_argv[new_argc++] = (*argv)[i];
    }
  }
  
  *argc = new_argc;
  return true;
}

// Run file directly (default behavior)
static cli_result_t run_file_direct(const char *file, int script_argc, char **script_argv, cli_context_t *ctx) {
  if (!cli_file_exists(file)) {
    fprintf(stderr, "Error: File '%s' not found\n", file);
    return CLI_ERROR;
  }
  
  if (ctx->verbose) {
    printf("Running file: %s\n", file);
  }
  
  char *source = read_file(file);
  if (!source) {
    return CLI_ERROR;
  }

  set_args(script_argc, script_argv);
  result_t result = interpret(source, file);
  free(source);

  return (result == RESULT_OK) ? CLI_SUCCESS : CLI_ERROR;
}

int main(int argc, char **argv) {
#ifdef DEBUG
  struct sigaction sa;
  sa.sa_handler = (void (*)(int))segfault_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sigaction(SIGSEGV, &sa, NULL);
#endif

  // Initialize CLI context
  cli_context_t ctx = {
    .verbose = false,
    .failed = false
  };

  // Skip program name
  argc--;
  argv++;
  
  // Parse global arguments
  if (!parse_global_args(&argc, &argv, &ctx)) {
    return ctx.failed ? CLI_ERROR : CLI_SUCCESS;
  }
  
  // Initialize VM and random seed
  uint64_t seed = get_seed();
  mt_seed_u64(seed);
  init_vm();
  
  cli_result_t result = CLI_SUCCESS;
  
  // Determine what to do based on arguments
  if (argc == 0) {
    // No arguments - start REPL
    result = cli_repl(0, NULL, &ctx);
    
  } else if (argc == 1 && cli_looks_like_file(argv[0])) {
    // Single argument that looks like a file - run it
    result = run_file_direct(argv[0], 0, NULL, &ctx);
    
  } else if (cli_looks_like_file(argv[0])) {
    // First argument looks like a file, rest are script arguments
    result = run_file_direct(argv[0], argc - 1, argv + 1, &ctx);
    
  } else {
    // First argument doesn't look like a file - treat as subcommand
    result = run_subcommand(argc, argv, &ctx);
  }
  
  // Handle exit codes and cleanup
  int exit_code = (result == CLI_SUCCESS) ? vm.exit_code : result;
  
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