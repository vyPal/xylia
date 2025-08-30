#ifndef XYL_DEBUG_H
#define XYL_DEBUG_H

#include "chunk.h"

#define DEBUG_LOG(...)                                                         \
  printf("[DEBUG] ");                                                          \
  printf(__VA_ARGS__)

void disassemble_chunk(chunk_t *chunk, const char *name);
int disassemble_instruction(chunk_t *chunk, int offset);

#endif
