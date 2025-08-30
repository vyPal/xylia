#include "chunk.h"
#include "memory.h"
#include "value.h"
#include "vm.h"

void init_chunk(chunk_t *chunk) {
  chunk->capacity = 0;
  chunk->count = 0;
  chunk->code = NULL;
  init_value_array(&chunk->constants);
}

void free_chunk(chunk_t *chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  free_value_array(&chunk->constants);
  init_chunk(chunk);
}

void write_chunk(chunk_t *chunk, uint8_t byte) {
  if (chunk->count >= chunk->capacity) {
    int old_capacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(old_capacity);
    chunk->code =
        GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
  }

  chunk->code[chunk->count++] = byte;
}

unsigned int add_constant(chunk_t *chunk, value_t value) {
  push(value);
  write_value_array(&chunk->constants, value);
  pop();
  return chunk->constants.count - 1;
}

void write_constant(uint8_t op, chunk_t *chunk, value_t value) {
  unsigned int constant = add_constant(chunk, value);
  if (constant > UINT8_MAX) {
    write_chunk(chunk, op + 1);
    write_chunk(chunk, constant & 0xff);
    write_chunk(chunk, (constant >> 8) & 0xff);
    write_chunk(chunk, (constant >> 16) & 0xff);
  } else {
    write_chunk(chunk, op);
    write_chunk(chunk, constant);
  }
}
