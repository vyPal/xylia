#include <stdbool.h>
#include <string.h>

#include "scanner.h"

typedef struct {
  const char *start;
  const char *current;
  int row, col;
} scanner_t;

scanner_t scanner;

void init_scanner(const char *source) {
  scanner.start = source;
  scanner.current = source;
  scanner.row = 1;
  scanner.col = 1;
}

static bool is_alpha(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_digit(char c) {
  return '0' <= c && c <= '9';
}

static bool is_at_end(void) {
  return *scanner.current == '\0';
}

static char advance(void) {
  scanner.current++;
  scanner.col++;
  return scanner.current[-1];
}

static char peek(void) {
  return *scanner.current;
}

static char peek_next(void) {
  if (is_at_end())
    return '\0';
  return scanner.current[1];
}

static bool match(char expected) {
  if (is_at_end())
    return false;
  if (*scanner.current != expected)
    return false;
  scanner.current++;
  return true;
}

static token_t make_token(token_type_t type) {
  token_t token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.row = scanner.row;
  token.col = scanner.col;
  return token;
}

static token_t error_token(const char *msg) {
  token_t token;
  token.type = TOK_ERROR;
  token.start = msg;
  token.length = strlen(msg);
  token.row = scanner.row;
  token.col = scanner.col;
  return token;
}

static void skip_whitespace(void) {
  while (true) {
    switch (peek()) {
    case '\n':
      scanner.row++;
      scanner.col = 0;
      // fallthrough
    case ' ':
    case '\t':
    case '\r':
      advance();
      break;
    case '-':
      if (peek_next() == '-') {
        while (peek() != '\n' && !is_at_end())
          advance();
      } else
        return;
      break;
    default:
      return;
    }
  }
}

static token_type_t check_keyword(int start, int length, const char *rest,
                                  token_type_t type) {
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length) == 0)
    return type;
  return TOK_IDENT;
}

static token_type_t ident_type(void) {
  switch (scanner.start[0]) {
  case 'a':
    return check_keyword(1, 5, "ssert", TOK_ASSERT);
  case 'b':
    return check_keyword(1, 4, "reak", TOK_BREAK);
  case 'c':
    if (scanner.current - scanner.start > 1)
      switch (scanner.start[1]) {
      case 'l':
        return check_keyword(2, 3, "ass", TOK_CLASS);
      case 'o':
        return check_keyword(2, 6, "ntinue", TOK_CONTINUE);
      }
    break;
  case 'e':
    if (scanner.current - scanner.start > 1)
      switch (scanner.start[1]) {
      case 'l':
        return check_keyword(2, 2, "se", TOK_ELSE);
      case 'n':
        return check_keyword(2, 2, "um", TOK_ENUM);
      }
    break;
  case 'f':
    if (scanner.current - scanner.start > 1)
      switch (scanner.start[1]) {
      case 'a':
        return check_keyword(2, 3, "lse", TOK_FALSE);
      case 'o':
        return check_keyword(2, 1, "r", TOK_FOR);
      case 'u':
        return check_keyword(2, 2, "nc", TOK_FUNC);
      }
    break;
  case 'i':
    return check_keyword(1, 1, "f", TOK_IF);
  case 'l':
    return check_keyword(1, 2, "et", TOK_LET);
  case 'n':
    return check_keyword(1, 2, "il", TOK_NIL);
  case 'o':
    return check_keyword(1, 7, "perator", TOK_OPERATOR);
  case 'r':
    return check_keyword(1, 5, "eturn", TOK_RETURN);
  case 's':
    if (scanner.current - scanner.start > 1)
      switch (scanner.start[1]) {
      case 'e':
        return check_keyword(2, 2, "lf", TOK_SELF);
      case 'u':
        return check_keyword(2, 3, "per", TOK_SUPER);
      }
    break;
  case 't':
    return check_keyword(1, 3, "rue", TOK_TRUE);
  case 'u':
    return check_keyword(1, 4, "nary", TOK_UNARY);
  case 'w':
    return check_keyword(1, 4, "hile", TOK_WHILE);
  }

  return TOK_IDENT;
}

static token_t ident(void) {
  while (is_alpha(peek()) || is_digit(peek()))
    advance();
  return make_token(ident_type());
}

static token_t number(void) {
  while (is_digit(peek()))
    advance();

  if (peek() == '.' && is_digit(peek_next())) {
    do {
      advance();
    } while (is_digit(peek()));

    return make_token(TOK_FLOAT);
  }

  return make_token(TOK_NUMBER);
}

static token_t string(void) {
  while (peek() != '"' && !is_at_end()) {
    if (peek() == '\n') {
      scanner.col = 0;
      scanner.row++;
      return error_token("Unterminated string");
    }

    if (peek() == '\\')
      advance();
    advance();
  }

  if (is_at_end())
    return error_token("Unterminated string");

  advance();
  return make_token(TOK_STRING);
}

token_t scan_token(void) {
  skip_whitespace();

  scanner.start = scanner.current;
  if (is_at_end())
    return make_token(TOK_EOF);

  char c = advance();

  if (is_alpha(c))
    return ident();
  if (is_digit(c))
    return number();

  switch (c) {
  case '(':
    return make_token(TOK_LPAREN);
  case ')':
    return make_token(TOK_RPAREN);
  case '{':
    return make_token(TOK_LBRACE);
  case '}':
    return make_token(TOK_RBRACE);
  case '[':
    return make_token(TOK_LBRACKET);
  case ']':
    return make_token(TOK_RBRACKET);
  case ',':
    return make_token(TOK_COMMA);
  case '.':
    return make_token(match('.') ? TOK_SPREAD : TOK_DOT);
  case ':':
    return make_token(match(':') ? TOK_ACCESS : TOK_COLON);
  case ';':
    return make_token(TOK_SEMICOLON);
  case '=':
    return make_token(match('=') ? TOK_EQ : TOK_ASSIGN);
  case '+':
    return make_token(TOK_PLUS);
  case '-':
    return make_token(TOK_MINUS);
  case '*':
    return make_token(TOK_ASTERISK);
  case '/':
    return make_token(TOK_SLASH);
  case '%':
    return make_token(TOK_PERCENT);
  case '&':
    return make_token(match('&') ? TOK_LOG_AND : TOK_BIT_AND);
  case '|':
    return make_token(match('|') ? TOK_LOG_OR : TOK_BIT_OR);
  case '~':
    return make_token(TOK_BIT_NOT);
  case '^':
    return make_token(TOK_GRAVE);
  case '!':
    return make_token(match('=') ? TOK_NEQ : TOK_LOG_NOT);
  case '<':
    if (match('<'))
      return make_token(TOK_SHIFTL);
    return make_token(match('=') ? TOK_LE : TOK_LT);
  case '>':
    if (match('>'))
      return make_token(TOK_SHIFTR);
    return make_token(match('=') ? TOK_GE : TOK_GT);
  case '"':
    return string();
  }

  return error_token("Unexpected character");
}
