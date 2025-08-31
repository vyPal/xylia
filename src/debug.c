#include <stdint.h>
#include <stdio.h>

#include "chunk.h"
#include "debug.h"
#include "object.h"
#include "value.h"

void disassemble_chunk(chunk_t *chunk, const char *name) {
  printf("== %s ==\n", name);
  for (int offset = 0; offset < chunk->count;)
    offset = disassemble_instruction(chunk, offset);
}

static int simple_op(const char *name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

static int byte_op(const char *name, chunk_t *chunk, int offset) {
  uint8_t slot = chunk->code[offset + 1];
  printf("%-20s %8d\n", name, slot);
  return offset + 2;
}

static int long_op(const char *name, chunk_t *chunk, int offset) {
  uint8_t nib1 = chunk->code[offset + 1];
  uint8_t nib2 = chunk->code[offset + 2];
  uint8_t nib3 = chunk->code[offset + 3];

  unsigned int slot = nib1 | (nib2 << 8) | (nib3 << 16);

  printf("%-20s %8d\n", name, slot);
  return offset + 4;
}

static int constant_op(const char *name, chunk_t *chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];

  printf("%-20s %8d '", name, constant);
  print_value(chunk->constants.values[constant], true);
  printf("'\n");
  return offset + 2;
}

static int constant_op_long(const char *name, chunk_t *chunk, int offset) {
  uint8_t nib1 = chunk->code[offset + 1];
  uint8_t nib2 = chunk->code[offset + 2];
  uint8_t nib3 = chunk->code[offset + 3];

  unsigned int constant = nib1 | (nib2 << 8) | (nib3 << 16);

  printf("%-20s %8d '", name, constant);
  print_value(chunk->constants.values[constant], true);
  printf("'\n");
  return offset + 4;
}

static int invoke_op(const char *name, chunk_t *chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  uint8_t argc = chunk->code[offset + 2];

  printf("%-20s (%d args) %8d '", name, argc, constant);
  print_value(chunk->constants.values[constant], true);
  printf("'\n");
  return offset + 3;
}

static int invoke_op_long(const char *name, chunk_t *chunk, int offset) {
  uint8_t nib1 = chunk->code[offset + 1];
  uint8_t nib2 = chunk->code[offset + 2];
  uint8_t nib3 = chunk->code[offset + 3];

  unsigned int constant = nib1 | (nib2 << 8) | (nib3 << 16);
  uint8_t argc = chunk->code[offset + 4];

  printf("%-20s (%d args) %8d '", name, argc, constant);
  print_value(chunk->constants.values[constant], true);
  printf("'\n");
  return offset + 5;
}

static int jump_op(const char *name, int sign, chunk_t *chunk, int offset) {
  uint16_t jump = chunk->code[offset + 1] | (chunk->code[offset + 2] << 8);
  printf("%-20s %8d -> %d\n", name, offset, offset + 3 + sign * jump);
  return offset + 3;
}

int disassemble_instruction(chunk_t *chunk, int offset) {
  printf("%04d ", offset);

  op_code_t op = chunk->code[offset];
  switch (op) {
  case OP_CONSTANT:
    return constant_op("OP_CONSTANT", chunk, offset);
  case OP_CONSTANT_LONG:
    return constant_op_long("OP_CONSTANT_LONG", chunk, offset);
  case OP_DEFINE_GLOBAL:
    return constant_op("OP_DEFINE_GLOBAL", chunk, offset);
  case OP_DEFINE_GLOBAL_LONG:
    return constant_op_long("OP_DEFINE_GLOBAL_LONG", chunk, offset);
  case OP_GET_GLOBAL:
    return constant_op("OP_GET_GLOBAL", chunk, offset);
  case OP_GET_GLOBAL_LONG:
    return constant_op_long("OP_GET_GLOBAL_LONG", chunk, offset);
  case OP_SET_GLOBAL:
    return constant_op("OP_SET_GLOBAL", chunk, offset);
  case OP_SET_GLOBAL_LONG:
    return constant_op_long("OP_SET_GLOBAL_LONG", chunk, offset);
  case OP_GET_LOCAL:
    return byte_op("OP_GET_LOCAL", chunk, offset);
  case OP_GET_LOCAL_LONG:
    return long_op("OP_GET_LOCAL_LONG", chunk, offset);
  case OP_SET_LOCAL:
    return byte_op("OP_SET_LOCAL", chunk, offset);
  case OP_SET_LOCAL_LONG:
    return long_op("OP_SET_LOCAL_LONG", chunk, offset);
  case OP_GET_UPVALUE:
    return byte_op("OP_GET_UPVALUE", chunk, offset);
  case OP_GET_UPVALUE_LONG:
    return long_op("OP_GET_UPVALUE_LONG", chunk, offset);
  case OP_SET_UPVALUE:
    return byte_op("OP_SET_UPVALUE", chunk, offset);
  case OP_SET_UPVALUE_LONG:
    return long_op("OP_SET_UPVALUE_LONG", chunk, offset);
  case OP_GET_SUPER:
    return constant_op("OP_GET_SUPER", chunk, offset);
  case OP_GET_SUPER_LONG:
    return constant_op_long("OP_GET_SUPER_LONG", chunk, offset);
  case OP_GET_PROPERTY:
    return constant_op("OP_GET_PROPERTY", chunk, offset);
  case OP_GET_PROPERTY_LONG:
    return constant_op_long("OP_GET_PORPERTY_LONG", chunk, offset);
  case OP_SET_PROPERTY:
    return constant_op("OP_SET_PROPERTY", chunk, offset);
  case OP_SET_PROPERTY_LONG:
    return constant_op_long("OP_SET_PROPERTY_LONG", chunk, offset);
  case OP_GET_INDEX:
    return simple_op("OP_GET_INDEX", offset);
  case OP_SET_INDEX:
    return simple_op("OP_SET_INDEX", offset);
  case OP_INVOKE:
    return invoke_op("OP_INVOKE", chunk, offset);
  case OP_INVOKE_LONG:
    return invoke_op_long("OP_INVOKE_LONG", chunk, offset);
  case OP_SUPER_INVOKE:
    return invoke_op("OP_SUPER_INVOKE", chunk, offset);
  case OP_SUPER_INVOKE_LONG:
    return invoke_op_long("OP_SUPER_INVOKE_LONG", chunk, offset);
  case OP_VECTOR:
    return byte_op("OP_VECTOR", chunk, offset);
  case OP_VECTOR_LONG:
    return long_op("OP_VECTOR_LONG", chunk, offset);
  case OP_CLASS:
    return constant_op("OP_CLASS", chunk, offset);
  case OP_CLASS_LONG:
    return constant_op_long("OP_CLASS_LONG", chunk, offset);
  case OP_CLOSURE: {
    offset++;
    int constant = chunk->code[offset++];
    printf("%-20s %8d ", "OP_CLOSURE", constant);
    print_value(chunk->constants.values[constant], true);
    printf("\n");

    obj_function_t *function = AS_FUNCTION(chunk->constants.values[constant]);
    for (int j = 0; j < function->upvalue_count; j++) {
      int is_local = chunk->code[offset++];
      int index = chunk->code[offset++];
      printf("%04d      |                     %s %d\n", offset - 2,
             is_local ? "local" : "upvalue", index);
    }

    return offset;
  }
  case OP_CLOSURE_LONG: {
    offset++;
    int constant = (chunk->code[offset]) | (chunk->code[offset + 1] << 8) |
                   (chunk->code[offset + 2] << 16);
    offset += 3;
    printf("%-20s %8d ", "OP_CLOSURE_LONG", constant);
    print_value(chunk->constants.values[constant], true);
    printf("\n");

    obj_function_t *function = AS_FUNCTION(chunk->constants.values[constant]);
    for (int j = 0; j < function->upvalue_count; j++) {
      int is_local = chunk->code[offset++];
      int index = chunk->code[offset++];
      printf("%04d      |                     %s %d\n", offset - 2,
             is_local ? "local" : "upvalue", index);
    }

    return offset;
  }
  case OP_METHOD:
    return constant_op("OP_METHOD", chunk, offset);
  case OP_METHOD_LONG:
    return constant_op_long("OP_METHOD_LONG", chunk, offset);
  case OP_TRUE:
    return simple_op("OP_TRUE", offset);
  case OP_FALSE:
    return simple_op("OP_FALSE", offset);
  case OP_NIL:
    return simple_op("OP_NIL", offset);
  case OP_POP:
    return simple_op("OP_POP", offset);
  case OP_ADD:
    return simple_op("OP_ADD", offset);
  case OP_SUB:
    return simple_op("OP_SUB", offset);
  case OP_MUL:
    return simple_op("OP_MUL", offset);
  case OP_DIV:
    return simple_op("OP_DIV", offset);
  case OP_MOD:
    return simple_op("OP_MOD", offset);
  case OP_BIT_AND:
    return simple_op("OP_BIT_AND", offset);
  case OP_BIT_OR:
    return simple_op("OP_BIT_OR", offset);
  case OP_XOR:
    return simple_op("OP_XOR", offset);
  case OP_EQ:
    return simple_op("OP_EQ", offset);
  case OP_GT:
    return simple_op("OP_GT", offset);
  case OP_GE:
    return simple_op("OP_GE", offset);
  case OP_LT:
    return simple_op("OP_LT", offset);
  case OP_LE:
    return simple_op("OP_LE", offset);
  case OP_NEG:
    return simple_op("OP_NEG", offset);
  case OP_LOG_NOT:
    return simple_op("OP_LOG_NOT", offset);
  case OP_BIT_NOT:
    return simple_op("OP_BIT_NOT", offset);
  case OP_CLOSE_UPVALUE:
    return simple_op("OP_CLOSE_UPVALUE", offset);
  case OP_INHERIT:
    return simple_op("OP_INHERIT", offset);
  case OP_ASSERT:
    return simple_op("OP_ASSERT", offset);
  case OP_ASSERT_MSG:
    return simple_op("OP_ASSERT_MSG", offset);
  case OP_CALL:
    return byte_op("OP_CALL", chunk, offset);
  case OP_LOOP:
    return jump_op("OP_LOOP", -1, chunk, offset);
  case OP_JUMP:
    return jump_op("OP_JUMP", 1, chunk, offset);
  case OP_JUMP_IF_FALSE:
    return jump_op("JUMP_IF_FALSE", 1, chunk, offset);
  case OP_RETURN:
    return simple_op("OP_RETURN", offset);
  case OP_LIST:
    return byte_op("OP_LIST", chunk, offset);
  case OP_LIST_LONG:
    return long_op("OP_LIST_LONG", chunk, offset);
  case OP_SPREAD:
    return simple_op("OP_SPREAD", offset);
  case OP_RANGE:
    return simple_op("OP_RANGE", offset);
  default:
    printf("Unknown opcode 0x%02x\n", op);
    return offset + 1;
  }
}
