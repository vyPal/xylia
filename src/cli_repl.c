#include <stdio.h>

#include "cli.h"
#include "repl.h"
#include "vm.h"

// Implementation of the 'repl' subcommand
cli_result_t cli_repl(int argc, char **argv, cli_context_t *ctx) {
  // REPL doesn't need any arguments, but warn if provided
  if (argc > 0) {
    fprintf(stderr, "Warning: 'repl' command ignores additional arguments\n");
  }

  if (ctx->verbose) {
    printf("Starting Xylia REPL...\n");
  }

  // Set empty args for REPL
  set_args(0, NULL);

  // Start the REPL
  run_repl();

  return ctx->failed ? CLI_ERROR : CLI_SUCCESS;
}
