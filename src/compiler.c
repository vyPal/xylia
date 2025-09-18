#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "scanner.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#ifdef DECOMPILE
#include "debug.h"
#endif

typedef struct {
  token_t current;
  token_t previous;
  bool had_error;
  bool panic_mode;
} parser_t;

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_RANGE,      // :
  PREC_OR,         // ||
  PREC_AND,        // &&
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_BIN_OR,     // |
  PREC_XOR,        // ^
  PREC_BIN_AND,    // &
  PREC_SHIFT,      // << >>
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // ! - ~ ..
  PREC_CALL,       // . () []
  PREC_PRIMARY,
} precedence_t;

typedef enum {
  TYPE_FUNCTION,
  TYPE_INITIALIZER,
  TYPE_METHOD,
  TYPE_SCRIPT,
} function_type_t;

typedef void (*parse_fn_t)(bool can_assign);

typedef struct {
  parse_fn_t prefix;
  parse_fn_t infix;
  precedence_t precedence;
} parse_rule_t;

typedef struct {
  token_t name;
  int depth;
  bool is_captured;
} local_t;

typedef struct {
  uint8_t index;
  bool is_local;
} upvalue_t;

typedef struct compiler {
  struct compiler *enclosing;
  obj_string_t *path;
  obj_function_t *function;
  function_type_t type;
  table_t *globals;

  local_t *locals;
  int local_capacity;
  int local_count;

  upvalue_t upvalues[UINT8_MAX + 1];
  int upvalue_capacity;

  int scope_depth;
} compiler_t;

typedef struct class_compiler {
  struct class_compiler *enclosing;
  bool has_superclass;
} class_compiler_t;

typedef struct loop {
  struct loop *next;
  int start;
  int break_addr[512];
  int break_count;
} loop_t;

parser_t parser;
compiler_t *current = NULL;
class_compiler_t *current_class = NULL;
chunk_t *compiling_chunk = NULL;
loop_t *current_loop = NULL;

static void init_compiler(compiler_t *compiler, obj_string_t *path,
                          function_type_t type, table_t *globals) {
  compiler->enclosing = current;
  compiler->path = path;
  compiler->function = NULL;
  compiler->type = type;
  compiler->local_capacity = 8;
  compiler->local_count = 0;
  compiler->locals = NULL;
  compiler->locals =
      GROW_ARRAY(local_t, compiler->locals, 0, compiler->local_capacity);
  compiler->scope_depth = 0;
  compiler->function = new_function();
  compiler->globals = globals;
  current = compiler;
  if (type != TYPE_SCRIPT)
    current->function->name =
        copy_string(parser.previous.start, parser.previous.length, true);
  compiler->function->path = path;
  compiler->function->row = parser.previous.row;
  compiler->function->col = parser.previous.col;

  local_t *local = &current->locals[current->local_count++];
  local->depth = 0;
  local->is_captured = false;
  if (type == TYPE_FUNCTION) {
    local->name.start = "";
    local->name.length = 0;
  } else {
    local->name.start = "self";
    local->name.length = 4;
  }
}

static chunk_t *current_chunk(void) {
  return &current->function->chunk;
}

static void error_at(token_t *token, const char *msg) {
  if (parser.panic_mode)
    return;

  parser.panic_mode = true;
  fprintf(stderr, "Error in '%s' at row:%d col:%d", current->path->chars,
          token->row, token->col);

  if (token->type == TOK_EOF)
    fprintf(stderr, " at end");
  else if (token->type != TOK_ERROR)
    fprintf(stderr, " at '%.*s'", token->length, token->start);

  fprintf(stderr, ": %s\n", msg);
  parser.had_error = true;
}

static void error(const char *msg) {
  error_at(&parser.previous, msg);
}

static void error_at_current(const char *msg) {
  error_at(&parser.current, msg);
}

static void advance(void) {
  parser.previous = parser.current;
  while (true) {
    parser.current = scan_token();
    if (parser.current.type != TOK_ERROR)
      break;
    error_at_current(parser.current.start);
  }
}

static void consume(token_type_t type, const char *msg) {
  if (parser.current.type == type)
    advance();
  else
    error_at_current(msg);
}

static bool check(token_type_t type) {
  return parser.current.type == type;
}

static bool match(token_type_t type) {
  if (!check(type))
    return false;
  advance();
  return true;
}

static void emit_byte(uint8_t byte) {
  write_chunk(current_chunk(), byte, parser.previous.row, parser.previous.col);
}

static void emit_bytes(uint8_t byte1, uint8_t byte2) {
  emit_byte(byte1);
  emit_byte(byte2);
}

static void emit_loop(unsigned int loop_start) {
  emit_byte(OP_LOOP);

  int offset = current_chunk()->count - loop_start + 2;
  if (offset > UINT16_MAX)
    error("Loop body too large");

  emit_byte(offset & 0xff);
  emit_byte((offset >> 8) & 0xff);
}

static unsigned int emit_jump(op_code_t instruction) {
  emit_byte(instruction);
  emit_bytes(0xff, 0xff);
  return current_chunk()->count - 2;
}

static void patch_jump(unsigned int offset) {
  unsigned int jump = current_chunk()->count - offset - 2;
  if (jump > UINT16_MAX)
    error("Too much code to jump over");

  current_chunk()->code[offset] = jump & 0xff;
  current_chunk()->code[offset + 1] = (jump >> 8) & 0xff;
}

static void emit_return(void) {
  if (current->type == TYPE_INITIALIZER)
    emit_bytes(OP_GET_LOCAL, 0);
  else
    emit_byte(OP_NIL);
  emit_byte(OP_RETURN);
}

static obj_function_t *end_compiler(void) {
  emit_return();
  obj_function_t *function = current->function;
  function->globals = current->globals;

#ifdef DECOMPILE
  if (!parser.had_error)
    disassemble_chunk(current_chunk(), function->name != NULL
                                           ? function->name->chars
                                           : "<script>");
#endif

  FREE_ARRAY(local_t, current->locals, current->local_capacity);
  current = current->enclosing;
  return function;
}

static void begin_scope(void) {
  current->scope_depth++;
}

static void end_scope(void) {
  current->scope_depth--;

  while (current->local_count > 0 &&
         current->locals[current->local_count - 1].depth >
             current->scope_depth) {
    if (current->locals[current->local_count - 1].is_captured)
      emit_byte(OP_CLOSE_UPVALUE);
    else
      emit_byte(OP_POP);
    current->local_count--;
  }
}

static void emit_var_op(op_code_t op, unsigned int arg) {
  if (arg > UINT8_MAX) {
    emit_byte(op + 1);
    emit_byte(arg & 0xff);
    emit_byte((arg >> 8) & 0xff);
    emit_byte((arg >> 16) & 0xff);
  } else
    emit_bytes(op, arg);
}

static bool idents_equal(token_t *a, token_t *b) {
  if (a->length != b->length)
    return false;
  return memcmp(a->start, b->start, a->length) == 0;
}

static token_t synthetic_token(const char *str) {
  token_t token;
  token.start = str;
  token.length = strlen(str);
  return token;
}

static parse_rule_t *get_rule(token_type_t type);
static void parse_precedence(precedence_t prec);
static void expression(void);
static void block(void);
static void statement(void);

static void synchronize(void) {
  parser.panic_mode = false;

  while (parser.current.type != TOK_EOF) {
    if (parser.previous.type == TOK_SEMICOLON)
      return;

    switch (parser.current.type) {
    case TOK_CLASS:
    case TOK_FOR:
    case TOK_FUNC:
    case TOK_IF:
    case TOK_LET:
    case TOK_RETURN:
    case TOK_WHILE:
      return;

    default:; // Do nothing.
    }

    advance();
  }
}

static bool is_operator(token_type_t type) {
  switch (type) {
  case TOK_EQ:
  case TOK_GT:
  case TOK_GE:
  case TOK_LT:
  case TOK_LE:
  case TOK_PLUS:
  case TOK_MINUS:
  case TOK_ASTERISK:
  case TOK_SLASH:
  case TOK_PERCENT:
  case TOK_GRAVE:
  case TOK_BIT_OR:
  case TOK_BIT_AND:
  case TOK_BIT_NOT:
  case TOK_LOG_NOT:
    return true;
  default:
    return false;
  }
}

static const char *op_to_name(token_type_t type) {
  switch (type) {
  case TOK_EQ:
    return "__eq__";
  case TOK_GT:
    return "__gt__";
  case TOK_GE:
    return "__ge__";
  case TOK_LT:
    return "__lt__";
  case TOK_LE:
    return "__le__";
  case TOK_PLUS:
    return "__add__";
  case TOK_MINUS:
    return "__sub__";
  case TOK_ASTERISK:
    return "__mul__";
  case TOK_SLASH:
    return "__div__";
  case TOK_PERCENT:
    return "__mod__";
  case TOK_GRAVE:
    return "__xor__";
  case TOK_BIT_OR:
    return "__bit_or__";
  case TOK_BIT_AND:
    return "__bit_and__";
  case TOK_BIT_NOT:
    return "__bit_not__";
  case TOK_LOG_NOT:
    return "__log_not__";
  default: // Unreachable
    return "";
  }
}

static obj_string_t *op_to_func_name(token_type_t type) {
  switch (type) {
  case TOK_EQ:
    return copy_string("operator ==", 11, true);
  case TOK_GT:
    return copy_string("operator >", 10, true);
  case TOK_GE:
    return copy_string("operator >=", 11, true);
  case TOK_LT:
    return copy_string("operator <", 10, true);
  case TOK_LE:
    return copy_string("operator <=", 11, true);
  case TOK_PLUS:
    return copy_string("operator +", 10, true);
  case TOK_MINUS:
    return copy_string("operator -", 10, true);
  case TOK_ASTERISK:
    return copy_string("operator *", 10, true);
  case TOK_SLASH:
    return copy_string("operator /", 10, true);
  case TOK_PERCENT:
    return copy_string("operator %", 10, true);
  case TOK_GRAVE:
    return copy_string("operator ^", 10, true);
  case TOK_BIT_OR:
    return copy_string("operator |", 10, true);
  case TOK_BIT_AND:
    return copy_string("operator &", 10, true);
  case TOK_BIT_NOT:
    return copy_string("operator ~", 10, true);
  case TOK_LOG_NOT:
    return copy_string("operator !", 10, true);
  default: // Unreachable
    return copy_string("invalid operator", 16, true);
  }
}

static obj_string_t *op_overload(token_t *method) {
  if (!is_operator(parser.current.type)) {
    if (match(TOK_LBRACKET)) {
      if (match(TOK_COLON)) {
        consume(TOK_RBRACKET, "Expected ']' after ':' symbol");
        if (match(TOK_ASSIGN)) {
          method->start = "__set_slice__";
          method->length = 13;
          return copy_string("operator [:]=", 13, true);
        } else {
          method->start = "__get_slice__";
          method->length = 13;
          return copy_string("operator [:]", 12, true);
        }
      } else {
        consume(TOK_RBRACKET, "Expected ']' after '[' symbol");
        if (match(TOK_ASSIGN)) {
          method->start = "__set_index__";
          method->length = 13;
          return copy_string("operator []=", 12, true);
        } else {
          method->start = "__get_index__";
          method->length = 13;
          return copy_string("operator []", 11, true);
        }
      }
    } else if (match(TOK_UNARY)) {
      consume(TOK_MINUS, "Expected '-' after 'unary'");
      method->start = "__neg__";
      method->length = 7;
      return copy_string("operator unary-", 15, true);
    }
    error("Expected operator symbol after 'operator'");
    return copy_string("invalid operator", 16, true);
  }

  const char *op_name = op_to_name(parser.current.type);
  obj_string_t *func_name = op_to_func_name(parser.current.type);
  advance();

  method->start = op_name;
  method->length = strlen(op_name);
  return func_name;
}

static unsigned int ident_constant(token_t *name) {
  return add_constant(current_chunk(),
                      OBJ_VAL(copy_string(name->start, name->length, true)));
}

static uint8_t argument_list(void) {
  uint8_t count = 0;
  if (!check(TOK_RPAREN)) {
    do {
      expression();
      if (count >= UINT8_MAX)
        error("Can't have more than 255 arguments");
      count++;
    } while (match(TOK_COMMA));
  }

  consume(TOK_RPAREN, "Expected ')' after arguments");
  return count;
}

static void add_local(token_t name) {
  if (current->local_count >= current->local_capacity) {
    int old_capacity = current->local_capacity;
    current->local_capacity = GROW_CAPACITY(old_capacity);
    current->locals = GROW_ARRAY(local_t, current->locals, old_capacity,
                                 current->local_capacity);
  }

  local_t *local = &current->locals[current->local_count++];
  local->name = name;
  local->depth = -1;
  local->is_captured = false;
}

static int resolve_local(compiler_t *compiler, token_t *name) {
  for (int i = compiler->local_count - 1; i >= 0; i--) {
    local_t *local = &compiler->locals[i];
    if (idents_equal(name, &local->name)) {
      if (local->depth == -1)
        error("Can't read local variable in its own initializer");
      return i;
    }
  }
  return -1;
}

static unsigned int add_upvalue(compiler_t *compiler, int index,
                                bool is_local) {
  int upvalue_count = compiler->function->upvalue_count;

  for (int i = 0; i < upvalue_count; i++) {
    upvalue_t *upvalue = &compiler->upvalues[i];
    if (upvalue->index == index && upvalue->is_local == is_local)
      return i;
  }

  if (upvalue_count == UINT8_MAX + 1) {
    error("Too many closure variables in function");
    return 0;
  }

  compiler->upvalues[upvalue_count].is_local = is_local;
  compiler->upvalues[upvalue_count].index = index;
  return compiler->function->upvalue_count++;
}

static int resolve_upvalue(compiler_t *compiler, token_t *name) {
  if (compiler->enclosing == NULL)
    return -1;

  int local = resolve_local(compiler->enclosing, name);
  if (local != -1) {
    compiler->enclosing->locals[local].is_captured = true;
    return add_upvalue(compiler, local, true);
  }

  int upvalue = resolve_upvalue(compiler->enclosing, name);
  if (upvalue != -1)
    return add_upvalue(compiler, upvalue, false);

  return -1;
}

static void grouping(bool _) {
  expression();
  consume(TOK_RPAREN, "Expected ')' after expression");
}

static void literal(bool _) {
  switch (parser.previous.type) {
  case TOK_TRUE:
    emit_byte(OP_TRUE);
    break;
  case TOK_FALSE:
    emit_byte(OP_FALSE);
    break;
  case TOK_NIL:
    emit_byte(OP_NIL);
    break;
  default: // Unreachable
    return;
  }
}

static void binary(bool _) {
  token_type_t operator_type = parser.previous.type;
  parse_rule_t *rule = get_rule(operator_type);
  parse_precedence((precedence_t)(rule->precedence + 1));

  switch (operator_type) {
  case TOK_COLON:
    emit_byte(OP_RANGE);
    break;
  case TOK_PLUS:
    emit_byte(OP_ADD);
    break;
  case TOK_MINUS:
    emit_byte(OP_SUB);
    break;
  case TOK_ASTERISK:
    emit_byte(OP_MUL);
    break;
  case TOK_SLASH:
    emit_byte(OP_DIV);
    break;
  case TOK_PERCENT:
    emit_byte(OP_MOD);
    break;
  case TOK_BIT_AND:
    emit_byte(OP_BIT_AND);
    break;
  case TOK_BIT_OR:
    emit_byte(OP_BIT_OR);
    break;
  case TOK_GRAVE:
    emit_byte(OP_XOR);
    break;
  case TOK_EQ:
    emit_byte(OP_EQ);
    break;
  case TOK_NEQ:
    emit_bytes(OP_EQ, OP_LOG_NOT);
    break;
  case TOK_GT:
    emit_byte(OP_GT);
    break;
  case TOK_GE:
    emit_byte(OP_GE);
    break;
  case TOK_LT:
    emit_byte(OP_LT);
    break;
  case TOK_LE:
    emit_byte(OP_LE);
    break;
  default: // Unreachable
    return;
  }
}

static void unary(bool _) {
  token_type_t operator_type = parser.previous.type;
  parse_precedence(PREC_UNARY);

  switch (operator_type) {
  case TOK_SPREAD:
    emit_byte(OP_SPREAD);
    break;
  case TOK_MINUS:
    emit_byte(OP_NEG);
    break;
  case TOK_BIT_NOT:
    emit_byte(OP_BIT_NOT);
    break;
  case TOK_LOG_NOT:
    emit_byte(OP_LOG_NOT);
    break;
  default: // Unreachable
    return;
  }
}

static void named_variable(token_t name, bool can_assign) {
  op_code_t get_op, set_op;
  int arg = resolve_local(current, &name);
  if (arg != -1) {
    get_op = OP_GET_LOCAL;
    set_op = OP_SET_LOCAL;
  } else if ((arg = resolve_upvalue(current, &name)) != -1) {
    get_op = OP_GET_UPVALUE;
    set_op = OP_SET_UPVALUE;
  } else {
    arg = ident_constant(&name);
    get_op = OP_GET_GLOBAL;
    set_op = OP_SET_GLOBAL;
  }

  if (can_assign && match(TOK_ASSIGN)) {
    expression();
    emit_var_op(set_op, arg);
  } else
    emit_var_op(get_op, arg);
}

static void vector(bool _) {
  unsigned int count = 0;
  if (!check(TOK_RBRACE)) {
    do {
      expression();
      count++;
    } while (match(TOK_COMMA));
  }

  consume(TOK_RBRACE, "Expected '}' after vector literal");
  emit_var_op(OP_VECTOR, count);
}

static void list(bool _) {
  unsigned int count = 0;
  if (!check(TOK_RBRACKET)) {
    do {
      expression();
      count++;
    } while (match(TOK_COMMA));
  }

  consume(TOK_RBRACKET, "Expected ']' after list literal");
  emit_var_op(OP_LIST, count);
}

static void variable(bool can_assign) {
  named_variable(parser.previous, can_assign);
}

static void float_(bool _) {
  double value = strtod(parser.previous.start, NULL);
  write_constant(OP_CONSTANT, current_chunk(), FLOAT_VAL(value));
}

static void number(bool _) {
  int64_t value = strtoll(parser.previous.start, NULL, 10);
  write_constant(OP_CONSTANT, current_chunk(), NUMBER_VAL(value));
}

static void string(bool _) {
  const char *src = parser.previous.start + 1;
  int length = parser.previous.length - 2;

  char *buffer = ALLOCATE(char, length + 1);
  int out_len = 0;

  for (int i = 0; i < length; i++) {
    if (src[i] == '\\') {
      switch (src[++i]) {
      case 'n':
        buffer[out_len++] = '\n';
        break;
      case 't':
        buffer[out_len++] = '\t';
        break;
      case 'r':
        buffer[out_len++] = '\r';
        break;
      case 'b':
        buffer[out_len++] = '\b';
        break;
      case '"':
        buffer[out_len++] = '"';
        break;
      case '\\':
        buffer[out_len++] = '\\';
        break;
      case 'x': {
        if (i + 2 >= length) {
          error("Incomplete hex escape sequence");
          break;
        }

        char hex[3] = {src[i + 1], src[i + 2], '\0'};
        char *endptr;
        long val = strtol(hex, &endptr, 16);
        if (*endptr != '\0' || val < 0 || val > 255) {
          error("Invalid hex escape");
          break;
        }

        buffer[out_len++] = (char)val;
        i += 2;
      } break;
      case '0': {
        int val = 0;
        int digits = 0;
        while (digits < 3 && i + 1 < length && '0' <= src[i + 1] &&
               src[i + 1] <= '7') {
          val = val * 8 + (src[++i] - '0');
          digits++;
        }
        buffer[out_len++] = (char)val;
      } break;
      default:
        buffer[out_len++] = '\\';
        buffer[out_len++] = src[i];
        break;
      }
    } else
      buffer[out_len++] = src[i];
  }

  buffer[out_len] = '\0';

  write_constant(OP_CONSTANT, current_chunk(),
                 OBJ_VAL(copy_string(buffer, out_len, true)));

  FREE_ARRAY(char, buffer, length + 1);
}

static void self(bool _) {
  if (current_class == NULL) {
    error("Can't use 'self' outside of a class");
    return;
  }

  variable(false);
}

static void super(bool _) {
  if (current_class == NULL)
    error("Can't use 'super' outside of a class");
  else if (!current_class->has_superclass)
    error("Can't use 'super' in class with no superclass");

  consume(TOK_DOT, "Expected '.' after 'super'");
  consume(TOK_IDENT, "Expected superclass method name");
  unsigned int name = ident_constant(&parser.previous);

  named_variable(synthetic_token("self"), false);
  if (match(TOK_LPAREN)) {
    uint8_t argc = argument_list();
    named_variable(synthetic_token("super"), false);
    emit_var_op(OP_SUPER_INVOKE, name);
    emit_byte(argc);
  } else {
    named_variable(synthetic_token("super"), false);
    emit_var_op(OP_GET_SUPER, name);
  }
}

static void call(bool _) {
  uint8_t count = argument_list();
  emit_bytes(OP_CALL, count);
}

static void index_(bool can_assign) {
  expression();
  consume(TOK_RBRACKET, "Expected ']' after index expression");

  if (can_assign && match(TOK_ASSIGN)) {
    expression();
    emit_byte(OP_SET_INDEX);
  } else
    emit_byte(OP_GET_INDEX);
}

static void dot(bool can_assign) {
  consume(TOK_IDENT, "Expected property name after '.'");
  unsigned int name = ident_constant(&parser.previous);

  if (can_assign && match(TOK_ASSIGN)) {
    expression();
    emit_var_op(OP_SET_PROPERTY, name);
  } else if (match(TOK_LPAREN)) {
    uint8_t count = argument_list();
    emit_var_op(OP_INVOKE, name);
    emit_byte(count);
  } else
    emit_var_op(OP_GET_PROPERTY, name);
}

static void and_(bool _) {
  int end_jump = emit_jump(OP_JUMP_IF_FALSE);

  emit_byte(OP_POP);
  parse_precedence(PREC_AND);

  patch_jump(end_jump);
}

static void or_(bool _) {
  int else_jump = emit_jump(OP_JUMP_IF_FALSE);
  int end_jump = emit_jump(OP_JUMP);

  patch_jump(else_jump);
  emit_byte(OP_POP);

  parse_precedence(PREC_OR);
  patch_jump(end_jump);
}

parse_rule_t rules[] = {
    [TOK_LPAREN] = {grouping, call, PREC_CALL},
    [TOK_RPAREN] = {NULL, NULL, PREC_NONE},
    [TOK_LBRACE] = {vector, NULL, PREC_NONE},
    [TOK_RBRACE] = {NULL, NULL, PREC_NONE},
    [TOK_LBRACKET] = {list, index_, PREC_CALL},
    [TOK_RBRACKET] = {NULL, NULL, PREC_NONE},
    [TOK_COMMA] = {NULL, NULL, PREC_NONE},
    [TOK_DOT] = {NULL, dot, PREC_CALL},
    [TOK_COLON] = {NULL, binary, PREC_RANGE},
    [TOK_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOK_ASSIGN] = {NULL, NULL, PREC_NONE},
    [TOK_SPREAD] = {unary, NULL, PREC_UNARY},
    [TOK_PLUS] = {NULL, binary, PREC_TERM},
    [TOK_MINUS] = {unary, binary, PREC_TERM},
    [TOK_ASTERISK] = {NULL, binary, PREC_FACTOR},
    [TOK_SLASH] = {NULL, binary, PREC_FACTOR},
    [TOK_PERCENT] = {NULL, binary, PREC_FACTOR},
    [TOK_BIT_AND] = {NULL, binary, PREC_BIN_AND},
    [TOK_BIT_OR] = {NULL, binary, PREC_BIN_OR},
    [TOK_BIT_NOT] = {unary, NULL, PREC_TERM},
    [TOK_GRAVE] = {NULL, binary, PREC_XOR},
    [TOK_LOG_AND] = {NULL, and_, PREC_AND},
    [TOK_LOG_OR] = {NULL, or_, PREC_OR},
    [TOK_LOG_NOT] = {unary, NULL, PREC_TERM},
    [TOK_EQ] = {NULL, binary, PREC_EQUALITY},
    [TOK_NEQ] = {NULL, binary, PREC_EQUALITY},
    [TOK_GT] = {NULL, binary, PREC_COMPARISON},
    [TOK_GE] = {NULL, binary, PREC_COMPARISON},
    [TOK_LT] = {NULL, binary, PREC_COMPARISON},
    [TOK_LE] = {NULL, binary, PREC_COMPARISON},
    [TOK_IDENT] = {variable, NULL, PREC_NONE},
    [TOK_FLOAT] = {float_, NULL, PREC_NONE},
    [TOK_NUMBER] = {number, NULL, PREC_NONE},
    [TOK_STRING] = {string, NULL, PREC_NONE},
    [TOK_TRUE] = {literal, NULL, PREC_NONE},
    [TOK_FALSE] = {literal, NULL, PREC_NONE},
    [TOK_NIL] = {literal, NULL, PREC_NONE},
    [TOK_ASSERT] = {NULL, NULL, PREC_NONE},
    [TOK_BREAK] = {NULL, NULL, PREC_NONE},
    [TOK_CLASS] = {NULL, NULL, PREC_NONE},
    [TOK_CONTINUE] = {NULL, NULL, PREC_NONE},
    [TOK_ELSE] = {NULL, NULL, PREC_NONE},
    [TOK_FOR] = {NULL, NULL, PREC_NONE},
    [TOK_FUNC] = {NULL, NULL, PREC_NONE},
    [TOK_IF] = {NULL, NULL, PREC_NONE},
    [TOK_LET] = {NULL, NULL, PREC_NONE},
    [TOK_OPERATOR] = {NULL, NULL, PREC_NONE},
    [TOK_RETURN] = {NULL, NULL, PREC_NONE},
    [TOK_SELF] = {self, NULL, PREC_NONE},
    [TOK_SUPER] = {super, NULL, PREC_NONE},
    [TOK_UNARY] = {NULL, NULL, PREC_NONE},
    [TOK_WHILE] = {NULL, NULL, PREC_NONE},
    [TOK_ERROR] = {NULL, NULL, PREC_NONE},
    [TOK_EOF] = {NULL, NULL, PREC_NONE},
};

static parse_rule_t *get_rule(token_type_t type) {
  return &rules[type];
}

static void parse_precedence(precedence_t precedence) {
  advance();
  parse_fn_t prefix_rule = get_rule(parser.previous.type)->prefix;
  if (prefix_rule == NULL) {
    error("Expected expression");
    return;
  }

  bool can_assign = precedence <= PREC_ASSIGNMENT;
  prefix_rule(can_assign);

  while (precedence <= get_rule(parser.current.type)->precedence) {
    advance();
    parse_fn_t infix_rule = get_rule(parser.previous.type)->infix;
    infix_rule(can_assign);
  }

  if (can_assign && match(TOK_ASSIGN))
    error("Invalid assignment target");
}

static void expression(void) {
  parse_precedence(PREC_ASSIGNMENT);
}

static void expression_statement(void) {
  expression();
  consume(TOK_SEMICOLON, "Expected ';' after expression");
  emit_byte(OP_POP);
}

static void declare_variable(void) {
  if (current->scope_depth == 0)
    return;

  token_t *name = &parser.previous;
  for (int i = current->local_count - 1; i >= 0; i--) {
    local_t *local = &current->locals[i];
    if (local->depth != -1 && local->depth < current->scope_depth)
      break;

    if (idents_equal(name, &local->name))
      error("Already a variable with this name in this scope");
  }

  add_local(*name);
}

static unsigned int parse_variable(const char *err) {
  consume(TOK_IDENT, err);

  declare_variable();
  if (current->scope_depth > 0)
    return 0;

  return ident_constant(&parser.previous);
}

static void mark_initialized(void) {
  if (current->scope_depth == 0)
    return;
  current->locals[current->local_count - 1].depth = current->scope_depth;
}

static void define_variable(unsigned int global) {
  if (current->scope_depth > 0) {
    mark_initialized();
    return;
  }

  emit_var_op(OP_DEFINE_GLOBAL, global);
}

static void function(function_type_t type, obj_string_t *name) {
  compiler_t compiler;
  init_compiler(&compiler, current->path, type, current->globals);
  begin_scope();

  if (name != NULL)
    compiler.function->name = name;

  consume(TOK_LPAREN, "Expected '(' after function name");
  if (!check(TOK_RPAREN)) {
    do {
      current->function->arity++;
      if (current->function->arity > UINT8_MAX)
        error_at_current("Can't have more than 255 parameters");
      unsigned int constant = parse_variable("Expected parameter name");
      define_variable(constant);

      if (match(TOK_LBRACKET)) {
        consume(TOK_RBRACKET, "Expected ']' after '[' in argument list");
        current->function->has_varargs = true;
        break;
      }
    } while (match(TOK_COMMA));
  }

  consume(TOK_RPAREN, "Expected ')' after parameters");
  consume(TOK_LBRACE, "Expected '{' after function body");
  block();

  obj_function_t *function = end_compiler();
  write_constant(OP_CLOSURE, current_chunk(), OBJ_VAL(function));

  for (int i = 0; i < function->upvalue_count; i++) {
    emit_byte(compiler.upvalues[i].is_local ? 1 : 0);
    emit_byte(compiler.upvalues[i].index);
  }
}

static void method(void) {
  token_t name;
  obj_string_t *func_name = NULL;
  if (match(TOK_OPERATOR))
    func_name = op_overload(&name);
  else {
    consume(TOK_FUNC, "Expected method declaration");
    consume(TOK_IDENT, "Expected method name");
    name = parser.previous;
  }

  unsigned int constant = ident_constant(&name);

  function_type_t type = TYPE_METHOD;
  if (name.length == 4 && memcmp(name.start, "init", 4) == 0)
    type = TYPE_INITIALIZER;

  function(type, func_name);
  emit_var_op(OP_METHOD, constant);
}

static void class_declaration(void) {
  consume(TOK_IDENT, "Expected class name");
  token_t class_name = parser.previous;
  unsigned int name_constant = ident_constant(&parser.previous);
  declare_variable();

  emit_var_op(OP_CLASS, name_constant);
  define_variable(name_constant);

  class_compiler_t class_compiler;
  class_compiler.enclosing = current_class;
  class_compiler.has_superclass = false;
  current_class = &class_compiler;

  if (match(TOK_COLON)) {
    consume(TOK_IDENT, "Expected superclass name");
    variable(false);

    if (idents_equal(&class_name, &parser.previous))
      error("A class can't inherit from itself");

    begin_scope();
    add_local(synthetic_token("super"));
    define_variable(0);

    named_variable(class_name, false);
    emit_byte(OP_INHERIT);
    class_compiler.has_superclass = true;
  }

  named_variable(class_name, false);
  consume(TOK_LBRACE, "Expected '{' before class body");
  while (!check(TOK_RBRACE) && !check(TOK_EOF))
    method();
  consume(TOK_RBRACE, "Expected '}' after class body");
  emit_byte(OP_POP);

  if (class_compiler.has_superclass)
    end_scope();

  current_class = current_class->enclosing;
}

static void func_declaration(void) {
  unsigned int global = parse_variable("Expected function name");
  mark_initialized();
  function(TYPE_FUNCTION, NULL);
  define_variable(global);
}

static void var_declaration(void) {
  unsigned int global = parse_variable("Expected variable name");

  if (match(TOK_ASSIGN))
    expression();
  else
    emit_byte(OP_NIL);

  consume(TOK_SEMICOLON, "Expected ';' after variable declaration");
  define_variable(global);
}

static void declaration(void) {
  if (match(TOK_CLASS))
    class_declaration();
  else if (match(TOK_FUNC))
    func_declaration();
  else if (match(TOK_LET))
    var_declaration();
  else
    statement();

  if (parser.panic_mode)
    synchronize();
}

static void assert_statement(void) {
  int row = parser.previous.row;
  int col = parser.previous.col;

  expression();

  if (match(TOK_COMMA)) {
    expression();
    emit_byte(OP_ASSERT_MSG);
  } else
    emit_byte(OP_ASSERT);

  emit_byte(row & 0xff);
  emit_byte((row >> 8) & 0xff);
  emit_byte((row >> 16) & 0xff);

  emit_byte(col & 0xff);
  emit_byte((col >> 8) & 0xff);
  emit_byte((col >> 16) & 0xff);

  chunk_t *chunk = current_chunk();
  unsigned int constant = add_constant(chunk, OBJ_VAL(current->path));
  write_chunk(chunk, constant & 0xff, parser.previous.row, parser.previous.col);
  write_chunk(chunk, (constant >> 8) & 0xff, parser.previous.row,
              parser.previous.col);
  write_chunk(chunk, (constant >> 16) & 0xff, parser.previous.row,
              parser.previous.col);

  consume(TOK_SEMICOLON, "Expected ';' after assert statement");
}

static void break_statement(void) {
  if (current_loop == NULL) {
    error("Can't use 'break' outside of loop");
    return;
  }

  consume(TOK_SEMICOLON, "Expected ';' after 'break'");
  // TODO: maybe check for break addr overflow
  current_loop->break_addr[current_loop->break_count++] = emit_jump(OP_JUMP);
}

static void continue_statement(void) {
  if (current_loop == NULL) {
    error("Can't use 'continue' outside of loop");
    return;
  }

  consume(TOK_SEMICOLON, "Expected ';' after 'continue'");
  emit_byte(OP_LOOP);

  unsigned int offset = current_chunk()->count - current_loop->start + 2;
  if (offset > UINT16_MAX)
    error("Loop body too large");

  emit_byte(offset & 0xff);
  emit_byte((offset >> 8) & 0xff);
}

static void for_statement(void) {
  begin_scope();
  consume(TOK_LPAREN, "Expected '(' after 'for'");

  if (match(TOK_SEMICOLON))
    ; // No initializer
  else if (match(TOK_LET))
    var_declaration();
  else
    expression_statement();

  unsigned int loop_start = current_chunk()->count;
  int exit_jump = -1;
  if (!match(TOK_SEMICOLON)) {
    expression();
    consume(TOK_SEMICOLON, "Expected ';' after loop condition");
    exit_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
  }

  loop_t *loop = (loop_t *)malloc(sizeof(loop_t));
  loop->start = loop_start;
  loop->break_count = 0;
  loop->next = current_loop ? current_loop : NULL;
  current_loop = loop;

  if (!match(TOK_RPAREN)) {
    unsigned int body_jump = emit_jump(OP_JUMP);
    unsigned int increment_start = current_chunk()->count;
    expression();
    emit_byte(OP_POP);
    consume(TOK_RPAREN, "Expected ')' after for clause");

    emit_loop(loop_start);
    loop_start = increment_start;
    patch_jump(body_jump);
  }

  statement();
  emit_loop(loop_start);

  if (exit_jump != -1) {
    patch_jump(exit_jump);
    emit_byte(OP_POP);
  }

  for (int i = 0; i < loop->break_count; i++)
    patch_jump(loop->break_addr[i]);

  current_loop = current_loop->next;
  free(loop);

  end_scope();
}

static void if_statement(void) {
  consume(TOK_LPAREN, "Expected '(' after 'if'");
  expression();
  consume(TOK_RPAREN, "Expected ')' after condition");

  unsigned int then_jump = emit_jump(OP_JUMP_IF_FALSE);
  emit_byte(OP_POP);
  statement();

  unsigned int else_jump = emit_jump(OP_JUMP);
  patch_jump(then_jump);
  emit_byte(OP_POP);

  if (match(TOK_ELSE))
    statement();

  patch_jump(else_jump);
}

static void return_statement(void) {
  if (current->type == TYPE_SCRIPT)
    error("Can't return from top-level code");

  if (match(TOK_SEMICOLON))
    emit_return();
  else {
    if (current->type == TYPE_INITIALIZER)
      error("Can't return a value from an initializer");

    expression();
    consume(TOK_SEMICOLON, "Expected ';' after return value");
    emit_byte(OP_RETURN);
  }
}

static void while_statement(void) {
  unsigned int loop_start = current_chunk()->count;
  consume(TOK_LPAREN, "Expected '(' after 'while'");
  expression();
  consume(TOK_RPAREN, "Expected ')' after condition");

  loop_t *loop = (loop_t *)malloc(sizeof(loop_t));
  loop->start = loop_start;
  loop->break_count = 0;
  loop->next = current_loop ? current_loop : NULL;
  current_loop = loop;

  unsigned int exit_jump = emit_jump(OP_JUMP_IF_FALSE);
  emit_byte(OP_POP);

  statement();
  emit_loop(loop_start);

  patch_jump(exit_jump);
  emit_byte(OP_POP);

  for (int i = 0; i < loop->break_count; i++)
    patch_jump(loop->break_addr[i]);

  current_loop = current_loop->next;
  free(loop);
}

static void block(void) {
  while (!check(TOK_RBRACE) && !check(TOK_EOF))
    declaration();

  consume(TOK_RBRACE, "Expected '}' after block");
}

static void statement(void) {
  if (match(TOK_ASSERT))
    assert_statement();
  else if (match(TOK_BREAK))
    break_statement();
  else if (match(TOK_CONTINUE))
    continue_statement();
  else if (match(TOK_FOR))
    for_statement();
  else if (match(TOK_IF))
    if_statement();
  else if (match(TOK_RETURN))
    return_statement();
  else if (match(TOK_WHILE))
    while_statement();
  else if (match(TOK_LBRACE)) {
    begin_scope();
    block();
    end_scope();
  } else
    expression_statement();
}

obj_module_t *compile(const char *source, obj_string_t *path,
                      obj_string_t *file) {
  obj_module_t *module =
      new_module(path == NULL ? copy_string("main", 4, true) : path);
  push(OBJ_VAL(module));

  init_scanner(source);
  compiler_t compiler;
  init_compiler(&compiler, file, TYPE_SCRIPT, &module->globals);

  parser.had_error = false;
  parser.panic_mode = false;

  advance();

  while (!match(TOK_EOF))
    declaration();

  obj_function_t *function = end_compiler();
  push(OBJ_VAL(function));
  obj_closure_t *closure = new_closure(function);
  pop();
  pop();
  module->init = closure;
  return parser.had_error ? NULL : module;
}

void mark_compiler_roots(void) {
  compiler_t *compiler = current;
  while (compiler != NULL) {
    mark_object((obj_t *)compiler->function);
    compiler = compiler->enclosing;
  }
}
