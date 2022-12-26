#include <ctype.h>
#include <glms/lexer.h>
#include <glms/macros.h>
#include <stddef.h>
#include <string.h>

typedef struct {
  const char *pattern;
  GLMSTokenType type;
} GLMSTokenMap;

#define GLMSTOKM(p, t)                                                         \
  (GLMSTokenMap) { .pattern = p, .type = t }

#define GLMS_LEXER_TOKEN_MAP_LEN 14

const GLMSTokenMap GLMS_LEXER_TOKEN_MAP[GLMS_LEXER_TOKEN_MAP_LEN] = {
    GLMSTOKM("function", GLMS_TOKEN_TYPE_SPECIAL_FUNCTION),
    GLMSTOKM("return", GLMS_TOKEN_TYPE_SPECIAL_RETURN),
    GLMSTOKM("if", GLMS_TOKEN_TYPE_SPECIAL_IF),
    GLMSTOKM("else", GLMS_TOKEN_TYPE_SPECIAL_ELSE),
    GLMSTOKM("false", GLMS_TOKEN_TYPE_SPECIAL_FALSE),
    GLMSTOKM("true", GLMS_TOKEN_TYPE_SPECIAL_TRUE),
    GLMSTOKM("for", GLMS_TOKEN_TYPE_SPECIAL_FOR),
    GLMSTOKM("while", GLMS_TOKEN_TYPE_SPECIAL_WHILE),
    GLMSTOKM("string", GLMS_TOKEN_TYPE_SPECIAL_STRING),
    GLMSTOKM("number", GLMS_TOKEN_TYPE_SPECIAL_NUMBER),
    GLMSTOKM("struct", GLMS_TOKEN_TYPE_SPECIAL_STRUCT),
    GLMSTOKM("let", GLMS_TOKEN_TYPE_SPECIAL_LET),
    GLMSTOKM("const", GLMS_TOKEN_TYPE_SPECIAL_CONST),
    GLMSTOKM("typedef", GLMS_TOKEN_TYPE_SPECIAL_TYPEDEF)};

int glms_lexer_init(GLMSLexer *lexer, const char *source) {
  if (!lexer || !source)
    return 0;
  if (lexer->initialized)
    return 1;
  lexer->initialized = true;

  lexer->source = source;
  lexer->i = 0;
  lexer->length = strlen(lexer->source);
  lexer->c = lexer->source[lexer->i];

  return 1;
}

static char glms_lexer_peek(GLMSLexer *lexer, int i) {
  if ((lexer->i + i) >= lexer->length)
    return 0;

  return lexer->source[lexer->i + i];
}

#define GLMS_LEXER_HAS_WHITESPACE                                              \
  (lexer->c == ' ' || lexer->c == '\t' || lexer->c == '\n' ||                  \
   lexer->c == 10 || lexer->c == 13)

static int glms_lexer_advance(GLMSLexer *lexer) {
  if (lexer->i >= lexer->length)
    return 0;
  if (lexer->c == 0)
    return 0;
  lexer->i += 1;
  lexer->c = lexer->source[lexer->i];
  return lexer->c != 0;
}

static int glms_lexer_skip_whitespace(GLMSLexer *lexer) {
  while (lexer->c != 0 && GLMS_LEXER_HAS_WHITESPACE) {
    if (!glms_lexer_advance(lexer))
      return 0;
  }

  return lexer->c != 0 && lexer->i < lexer->length;
}

static void glms_lexer_parse_special_id(GLMSLexer *lexer, GLMSToken *out) {
  const char *value = glms_string_view_get_value(&out->value);

  GLMSTokenType type = out->type;

  for (int i = 0; i < GLMS_LEXER_TOKEN_MAP_LEN; i++) {
    GLMSTokenMap m = GLMS_LEXER_TOKEN_MAP[i];

    if (strcmp(value, m.pattern) == 0) {
      type = m.type;
      break;
    }
  }

  out->type = type;
}

static int glms_lexer_parse_id(GLMSLexer *lexer, GLMSToken *out) {
  out->value.length = 0;
  out->value.ptr = &lexer->source[lexer->i];

  while (isalnum(lexer->c) && lexer->c != 0) {
    glms_lexer_advance(lexer);
    out->value.length++;
  }

  out->type = GLMS_TOKEN_TYPE_ID;

  glms_lexer_parse_special_id(lexer, out);

  return 1;
}

static int glms_lexer_parse_string(GLMSLexer *lexer, GLMSToken *out) {
  out->value.length = 0;
  glms_lexer_advance(lexer);
  out->value.ptr = &lexer->source[lexer->i];

  while (lexer->c != '"' && lexer->c != 0) {
    glms_lexer_advance(lexer);
    out->value.length++;
  }

  glms_lexer_advance(lexer);
  out->type = GLMS_TOKEN_TYPE_STRING;

  return 1;
}

static int glms_lexer_parse_number(GLMSLexer *lexer, GLMSToken *out) {
  out->value.length = 0;
  out->value.ptr = &lexer->source[lexer->i];

  while (isdigit(lexer->c) && lexer->c != 0) {
    glms_lexer_advance(lexer);
    out->value.length++;
  }

  if (lexer->c == '.') {
    out->value.length++;
    glms_lexer_advance(lexer);

    while (isdigit(lexer->c) && lexer->c != 0) {
      glms_lexer_advance(lexer);
      out->value.length++;
    }
  }

  out->type = GLMS_TOKEN_TYPE_NUMBER;

  return 1;
}

int glms_lexer_next(GLMSLexer *lexer, GLMSToken *out) {
  if (!lexer || !out)
    return 0;
  if (!lexer->initialized)
    GLMS_WARNING_RETURN(0, stderr, "Lexer not initialized.\n");
  if (lexer->i >= lexer->length)
    return 0;
  if (lexer->c == 0)
    return 0;

  out->c = 0;
  out->value.ptr = 0;
  out->value.length = 0;
  out->type = GLMS_TOKEN_TYPE_EOF;

  while (GLMS_LEXER_HAS_WHITESPACE) {
    if (!glms_lexer_skip_whitespace(lexer))
      return 0;
  }

  switch (lexer->c) {
  case '{': {
    out->type = GLMS_TOKEN_TYPE_LBRACE;
  } break;
  case '}': {
    out->type = GLMS_TOKEN_TYPE_RBRACE;
  } break;
  case '[': {
    out->type = GLMS_TOKEN_TYPE_LBRACKET;
  } break;
  case ']': {
    out->type = GLMS_TOKEN_TYPE_RBRACKET;
  } break;
  case '(': {
    out->type = GLMS_TOKEN_TYPE_LPAREN;
  } break;
  case ')': {
    out->type = GLMS_TOKEN_TYPE_RPAREN;
  } break;
  case ';': {
    out->type = GLMS_TOKEN_TYPE_SEMI;
  } break;
  case ',': {
    out->type = GLMS_TOKEN_TYPE_COMMA;
  } break;
  case '.': {
    out->type = GLMS_TOKEN_TYPE_DOT;
  } break;
  case ':': {
    out->type = GLMS_TOKEN_TYPE_COLON;
  } break;
  case '+': {
    out->type = GLMS_TOKEN_TYPE_ADD;

    if (glms_lexer_peek(lexer, 1) == '=') {
      out->type = GLMS_TOKEN_TYPE_ADD_EQUALS;
      glms_lexer_advance(lexer);
    } else if (glms_lexer_peek(lexer, 1) == '+') {
      out->type = GLMS_TOKEN_TYPE_ADD_ADD;
      glms_lexer_advance(lexer);
    }
  } break;
  case '*': {
    out->type = GLMS_TOKEN_TYPE_MUL;
  } break;
  case '-': {
    out->type = GLMS_TOKEN_TYPE_SUB;

    if (glms_lexer_peek(lexer, 1) == '=') {
      out->type = GLMS_TOKEN_TYPE_SUB_EQUALS;
      glms_lexer_advance(lexer);
    } else if (glms_lexer_peek(lexer, 1) == '-') {
      out->type = GLMS_TOKEN_TYPE_SUB_SUB;
      glms_lexer_advance(lexer);
    }
  } break;
  case '/': {
    out->type = GLMS_TOKEN_TYPE_DIV;
  } break;
  case '%': {
    out->type = GLMS_TOKEN_TYPE_PERCENT;
  } break;
  case '=': {
    out->type = GLMS_TOKEN_TYPE_EQUALS;

    if (glms_lexer_peek(lexer, 1) == '=') {
      out->type = GLMS_TOKEN_TYPE_EQUALS_EQUALS;
      glms_lexer_advance(lexer);
    }
  } break;
  case '>': {
    out->type = GLMS_TOKEN_TYPE_GT;

    if (glms_lexer_peek(lexer, 1) == '=') {
      out->type = GLMS_TOKEN_TYPE_GTE;
      glms_lexer_advance(lexer);
    }
  } break;
  case '<': {
    out->type = GLMS_TOKEN_TYPE_LT;

    if (glms_lexer_peek(lexer, 1) == '=') {
      out->type = GLMS_TOKEN_TYPE_LTE;
      glms_lexer_advance(lexer);
    }
  } break;
  default: {
    if (lexer->c == '"') {
      return glms_lexer_parse_string(lexer, out);
    }
    if (isdigit(lexer->c)) {
      return glms_lexer_parse_number(lexer, out);
    }
    if (isalpha(lexer->c)) {
      return glms_lexer_parse_id(lexer, out);
    }
    GLMS_WARNING_RETURN(0, stderr, "Unexpected token `%c`.\n", lexer->c);
  }; break;
  }

  out->c = lexer->c;

  glms_lexer_advance(lexer);

  return 1;
}
