#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "repl.h"
#include "replxx.h"
#include "vm.h"

extern bool failed;

#define BUILTIN(name) "__builtin___" #name
#define BUILTIN_CLEAN(name) #name

static const char *builtin_funcs[] = {
    "assert",
    "break",
    "class",
    "continue",
    "else",
    "for",
    "func",
    "if",
    "return",
    "super",
    "let",
    "while",
    "operator",
    "unary",

    "true",
    "false",
    "nil",

    "self",

    // IO
    BUILTIN(print),
    BUILTIN(println),
    BUILTIN(printf),
    BUILTIN(input),

    BUILTIN(open),
    BUILTIN(close),
    BUILTIN(read),
    BUILTIN(write),

    // Vectors
    BUILTIN_CLEAN(len),
    BUILTIN(append),
    BUILTIN(pop),
    BUILTIN(insert),
    BUILTIN(remove),
    BUILTIN(slice),

    // Utils
    BUILTIN_CLEAN(typeof),
    BUILTIN_CLEAN(isinstance),
    BUILTIN_CLEAN(hasmethod),
    BUILTIN_CLEAN(getclass),
    BUILTIN_CLEAN(exit),
    BUILTIN_CLEAN(argv),
    BUILTIN(hash),
    BUILTIN_CLEAN(import),

    // Casts
    BUILTIN_CLEAN(string),
    BUILTIN_CLEAN(number),
    BUILTIN_CLEAN(float),
    BUILTIN_CLEAN(bool),
    BUILTIN_CLEAN(vector),
    BUILTIN_CLEAN(list),

    // Math
    BUILTIN(abs),
    BUILTIN(min),
    BUILTIN(max),

    BUILTIN(sin),
    BUILTIN(cos),
    BUILTIN(tan),
    BUILTIN(asin),
    BUILTIN(acos),
    BUILTIN(atan),
    BUILTIN(atan2),

    BUILTIN(sqrt),
    BUILTIN(pow),
    BUILTIN(log),
    BUILTIN(exp),

    BUILTIN(floor),
    BUILTIN(ceil),
    BUILTIN(round),

    // Random
    BUILTIN(random),
    BUILTIN(randomseed),

    // Time
    BUILTIN(now),
    BUILTIN(clock),
    BUILTIN(sleep),
    BUILTIN(localtime),
};
static const size_t builtin_count =
    sizeof(builtin_funcs) / sizeof(builtin_funcs[0]);

#undef BUILTIN_CLEAN
#undef BUILTIN

static bool is_identifier_start(char c) {
  return isalpha((unsigned char)c) || c == '_';
}

static bool is_identifier_char(char c) {
  return isalnum((unsigned char)c) || c == '_';
}

void xylia_hint(const char *input, struct replxx_hints *hints, int *start_index,
                ReplxxColor *color, void *user_data) {
  (void)user_data;

  int len = strlen(input);
  if (len == 0)
    return;

  int start = len - 1;
  while (start >= 0 && (isalnum((unsigned char)input[start]) ||
                        input[start] == '_' || input[start] == '.'))
    start--;
  start++;
  int word_len = len - start;
  if (word_len <= 0)
    return;

  *start_index = word_len;

  if (color)
    *color = REPLXX_COLOR_GRAY;

  for (int i = 0; i < builtin_count; i++) {
    const char *func = builtin_funcs[i];
    if (strncmp(func, input + start, word_len) == 0)
      replxx_add_hint(hints, func);
  }
}

static ReplxxColor classify_identifier(const char *word, int len) {
  static const char *keywords[] = {
      "assert", "break",  "class", "continue", "else",  "for",      "func",
      "if",     "return", "super", "let",      "while", "operator", "unary"};
  static const char *constants[] = {"true", "false", "nil"};
  static const char *special_keywords[] = {
      "import",   "len",  "typeof", "isinstance", "hasmethod",
      "getclass", "exit", "argv",   "string",     "number",
      "float",    "bool", "vector", "list"};

  if (len == 4 && strncmp(word, "self", 4) == 0)
    return REPLXX_COLOR_BRIGHTRED;

  if (len > 10 && strncmp(word, "__builtin__", 11) == 0)
    return REPLXX_COLOR_BRIGHTRED;

  for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
    if ((int)strlen(keywords[i]) == len && strncmp(word, keywords[i], len) == 0)
      return REPLXX_COLOR_BRIGHTMAGENTA;

  for (size_t i = 0; i < sizeof(special_keywords) / sizeof(special_keywords[0]);
       i++)
    if ((int)strlen(special_keywords[i]) == len &&
        strncmp(word, special_keywords[i], len) == 0)
      return REPLXX_COLOR_BRIGHTRED;

  for (size_t i = 0; i < sizeof(constants) / sizeof(constants[0]); i++)
    if ((int)strlen(constants[i]) == len &&
        strncmp(word, constants[i], len) == 0)
      return REPLXX_COLOR_YELLOW;

  return REPLXX_COLOR_DEFAULT;
}

void xylia_highlighter(const char *input, ReplxxColor *colors, int size,
                       void *user_data) {
  (void)user_data;

  int i = 0;
  while (i < size) {
    char c = input[i];

    if (c == '-' && i + 1 < size && input[i + 1] == '-') {
      for (; i < size && input[i] != '\n'; i++)
        colors[i] = REPLXX_COLOR_GRAY;
      continue;
    }

    if (c == '"') {
      colors[i++] = REPLXX_COLOR_BRIGHTGREEN;
      while (i < size && input[i] != '"') {
        if (input[i] == '\\' && i + 1 < size)
          colors[i++] = REPLXX_COLOR_BRIGHTGREEN;
        colors[i++] = REPLXX_COLOR_BRIGHTGREEN;
      }
      if (i < size)
        colors[i++] = REPLXX_COLOR_BRIGHTGREEN;
      continue;
    }

    if (isdigit((unsigned char)c)) {
      int start = i;
      while (i < size && isdigit((unsigned char)input[i]))
        i++;
      if (i < size && input[i] == '.' && (i + 1 < size) &&
          isdigit((unsigned char)input[i + 1])) {
        i++;
        while (i < size && isdigit((unsigned char)input[i]))
          i++;
      }
      for (int j = start; j < i; j++)
        colors[j] = REPLXX_COLOR_BRIGHTBLUE;
      continue;
    }

    if (is_identifier_start(c)) {
      int start = i;
      while (i < size && is_identifier_char(input[i]))
        i++;
      int len = i - start;

      ReplxxColor color = classify_identifier(&input[start], len);

      if (color != REPLXX_COLOR_BRIGHTRED) {
        int j = i;
        while (j < size && isspace((unsigned char)input[j]))
          j++;
        if (j < size && input[j] == '(')
          color = REPLXX_COLOR_BRIGHTCYAN;
      }

      for (int k = start; k < i; ++k)
        colors[k] = color;
      continue;
    }

    i++;
  }
}

void run_repl() {
  Replxx *repl = replxx_init();
  replxx_set_highlighter_callback(repl, xylia_highlighter, NULL);
  replxx_set_hint_callback(repl, xylia_hint, NULL);
  replxx_set_max_hint_rows(repl, 5);

  const char *line;
  while (true) {
    char *buffer = NULL;
    const char *prompt = ">>> ";

    while ((line = replxx_input(repl, prompt)) != NULL) {
      if (*line) {
        size_t line_len = strlen(line);
        char *copy = strdup(line);

        int line_continues = (line_len > 0 && copy[line_len - 1] == '\\');
        if (line_continues)
          copy[line_len - 1] = '\0';

        if (buffer) {
          size_t buf_len = strlen(buffer);
          char *new_buf = malloc(buf_len + strlen(copy) + 2);
          if (!new_buf) {
            fprintf(stderr, "Out of memory\n");
            free(buffer);
            free(copy);
            replxx_end(repl);
            return;
          }
          memcpy(new_buf, buffer, buf_len);
          new_buf[buf_len] = '\n';
          memcpy(new_buf + buf_len + 1, copy, strlen(copy) + 1);
          free(buffer);
          free(copy);
          buffer = new_buf;
        } else {
          buffer = copy;
        }

        size_t buf_len = strlen(buffer);
        int ends_block = (buf_len > 0 && buffer[buf_len - 1] == '{');

        if (!line_continues && !ends_block)
          break;

        prompt = "... ";
        continue;
      } else
        break;
    }

    if (!buffer)
      break;

    interpret(buffer, "stdin");
    free(buffer);

    if (vm.signal != SIG_NONE)
      break;
  }

  // const char *input;

  // while ((input = replxx_input(repl, ">> ")) != NULL) {

  //   if (*input)
  //     replxx_history_add(repl, input);
  //   else
  //     break;

  //   interpret(input, "stdin");
  //   if (vm.signal != SIG_NONE)
  //     break;
  // }

  replxx_end(repl);
}
