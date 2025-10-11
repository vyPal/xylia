#ifndef XYL_SCANNER_H
#define XYL_SCANNER_H

typedef enum {
  TOK_LPAREN,    // (
  TOK_RPAREN,    // )
  TOK_LBRACE,    // {
  TOK_RBRACE,    // }
  TOK_LBRACKET,  // [
  TOK_RBRACKET,  // ]
  TOK_COMMA,     // ,
  TOK_DOT,       // .
  TOK_COLON,     // :
  TOK_ACCESS,    // ::
  TOK_SEMICOLON, // ;
  TOK_ASSIGN,    // =
  TOK_SPREAD,    // ..

  // Binary operators
  TOK_PLUS,     // +
  TOK_MINUS,    // -
  TOK_ASTERISK, // *
  TOK_SLASH,    // /
  TOK_PERCENT,  // %
  TOK_SHIFTL,   // <<
  TOK_SHIFTR,   // >>
  TOK_ARROW,    // ->

  // Bitwise operators
  TOK_BIT_AND, // &
  TOK_BIT_OR,  // |
  TOK_BIT_NOT, // ~
  TOK_GRAVE,   // ^

  // Logical operators
  TOK_LOG_AND, // &&
  TOK_LOG_OR,  // ||
  TOK_LOG_NOT, // !

  // Equality operators
  TOK_EQ,  // ==
  TOK_NEQ, // !=

  // Comparison operators
  TOK_GT, // >
  TOK_GE, // >=
  TOK_LT, // <
  TOK_LE, // <=

  // Literals
  TOK_IDENT,
  TOK_FLOAT,
  TOK_NUMBER,
  TOK_STRING,
  TOK_TRUE,
  TOK_FALSE,
  TOK_NIL,

  // Keyword
  TOK_ASSERT,
  TOK_BREAK,
  TOK_CLASS,
  TOK_CONTINUE,
  TOK_ELSE,
  TOK_FOR,
  TOK_FUNC,
  TOK_IF,
  TOK_LET,
  TOK_OPERATOR,
  TOK_RETURN,
  TOK_SELF,
  TOK_SUPER,
  TOK_UNARY,
  TOK_WHILE,

  // Special tokens
  TOK_ERROR,
  TOK_EOF,
} token_type_t;

typedef struct {
  token_type_t type;
  const char *start;
  int length;
  int row, col;
} token_t;

void init_scanner(const char *source);
token_t scan_token(void);

#endif
