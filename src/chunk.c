#include "chunk.h"
#include "memory.h"
#include "value.h"
#include "vm.h"

void init_chunk(chunk_t *chunk) {
  chunk->capacity = 0;
  chunk->count = 0;
  chunk->code = NULL;
  chunk->pos_capacity = 0;
  chunk->pos_count = 0;
  chunk->positions = NULL;
  init_value_array(&chunk->constants);
}

void free_chunk(chunk_t *chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(srcpos_t, chunk->positions, chunk->pos_capacity);
  free_value_array(&chunk->constants);
  init_chunk(chunk);
}

static inline void write_chunk_internal(chunk_t *chunk, uint8_t byte) {
  if (chunk->count >= chunk->capacity) {
    int old_capacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(old_capacity);
    chunk->code =
        GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
  }

  chunk->code[chunk->count++] = byte;
}

void write_chunk(chunk_t *chunk, uint8_t byte, int row, int col) {
  write_chunk_internal(chunk, byte);
  if (chunk->pos_count == 0 ||
      chunk->positions[chunk->pos_count - 1].row != row ||
      chunk->positions[chunk->pos_count - 1].col != col) {
    if (chunk->pos_capacity <= chunk->pos_count) {
      int old_capacity = chunk->capacity;
      chunk->pos_capacity = GROW_CAPACITY(old_capacity);
      chunk->positions = GROW_ARRAY(srcpos_t, chunk->positions, old_capacity,
                                    chunk->pos_capacity);
    }

    chunk->positions[chunk->pos_count].offset = chunk->count - 1;
    chunk->positions[chunk->pos_count].row = row;
    chunk->positions[chunk->pos_count].col = col;
    chunk->pos_count++;
  }
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
    write_chunk_internal(chunk, op + 1);
    write_chunk_internal(chunk, constant & 0xff);
    write_chunk_internal(chunk, (constant >> 8) & 0xff);
    write_chunk_internal(chunk, (constant >> 16) & 0xff);
  } else {
    write_chunk_internal(chunk, op);
    write_chunk_internal(chunk, constant);
  }
}

srcpos_t chunk_get_srcpos(chunk_t *chunk, int offset) {
  int lo = 0;
  int hi = chunk->pos_count + 1;

  while (lo <= hi) {
    int mid = (lo + hi) / 2;
    if (chunk->positions[mid].offset > offset)
      hi = mid - 1;
    else
      lo = mid + 1;
  }

  return chunk->positions[hi >= 0 ? hi : 0];
}
