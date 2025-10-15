#ifndef XYL_CHUNK_T
#define XYL_CHUNK_T

#include <stdint.h>

#include "value.h"

typedef enum {
  OP_CONSTANT,
  OP_CONSTANT_LONG,
  OP_DEFINE_GLOBAL,
  OP_DEFINE_GLOBAL_LONG,
  OP_GET_GLOBAL,
  OP_GET_GLOBAL_LONG,
  OP_SET_GLOBAL,
  OP_SET_GLOBAL_LONG,
  OP_GET_LOCAL,
  OP_GET_LOCAL_LONG,
  OP_SET_LOCAL,
  OP_SET_LOCAL_LONG,
  OP_GET_UPVALUE,
  OP_GET_UPVALUE_LONG,
  OP_SET_UPVALUE,
  OP_SET_UPVALUE_LONG,
  OP_GET_SUPER,
  OP_GET_SUPER_LONG,
  OP_GET_PROPERTY,
  OP_GET_PROPERTY_LONG,
  OP_SET_PROPERTY,
  OP_SET_PROPERTY_LONG,
  OP_GET_ACCESS,
  OP_GET_ACCESS_LONG,
  OP_GET_INDEX,
  OP_SET_INDEX,
  OP_INVOKE,
  OP_INVOKE_LONG,
  OP_INVOKE_ACCESS,
  OP_INVOKE_ACCESS_LONG,
  OP_SUPER_INVOKE,
  OP_SUPER_INVOKE_LONG,
  OP_VECTOR,
  OP_VECTOR_LONG,
  OP_LIST,
  OP_LIST_LONG,
  OP_CLASS,
  OP_CLASS_LONG,
  OP_ENUM,
  OP_ENUM_LONG,
  OP_CLOSURE,
  OP_CLOSURE_LONG,
  OP_METHOD,
  OP_METHOD_LONG,
  OP_ENUM_VALUE,
  OP_ENUM_VALUE_LONG,
  OP_ENUM_VALUE_CUSTOM,
  OP_ENUM_VALUE_CUSTOM_LONG,

  OP_TRUE,
  OP_FALSE,
  OP_NIL,
  OP_POP,

  OP_SPREAD,
  OP_RANGE,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD,
  OP_SHIFTL,
  OP_SHIFTR,
  OP_BIT_AND,
  OP_BIT_OR,
  OP_XOR,
  OP_EQ,
  OP_GT,
  OP_GE,
  OP_LT,
  OP_LE,

  OP_NEG,
  OP_LOG_NOT,
  OP_BIT_NOT,

  OP_CLOSE_UPVALUE,
  OP_INHERIT,

  OP_ASSERT,
  OP_ASSERT_MSG,

  OP_CALL,

  OP_LOOP,
  OP_JUMP,
  OP_JUMP_IF_FALSE,

  OP_RETURN,
} op_code_t;

typedef struct {
  int offset;
  int row, col;
} srcpos_t;

typedef struct {
  int count;
  int capacity;
  uint8_t *code;
  value_array_t constants;

  int pos_count;
  int pos_capacity;
  srcpos_t *positions;
} chunk_t;

void init_chunk(chunk_t *chunk);
void free_chunk(chunk_t *chunk);
void write_chunk(chunk_t *chunk, uint8_t byte, int row, int col);
unsigned int add_constant(chunk_t *chunk, value_t value);
void write_constant(uint8_t op, chunk_t *chunk, value_t value);
srcpos_t chunk_get_srcpos(chunk_t *chunk, int offset);

#endif
