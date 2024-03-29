#include <ctype.h>
#include <glms/lexer.h>
#include <glms/macros.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "glms/token.h"

typedef struct {
  const char* pattern;
  GLMSTokenType type;
} GLMSTokenMap;

#define GLMSTOKM(p, t) \
  (GLMSTokenMap) { .pattern = p, .type = t }

#define GLMS_LEXER_TOKEN_MAP_LEN 46

const GLMSTokenMap GLMS_LEXER_TOKEN_MAP[GLMS_LEXER_TOKEN_MAP_LEN] = {
    GLMSTOKM("fdecl", GLMS_TOKEN_TYPE_SPECIAL_FDECL),
    GLMSTOKM("glsl", GLMS_TOKEN_TYPE_SPECIAL_GLSL),
    GLMSTOKM("buffer", GLMS_TOKEN_TYPE_SPECIAL_BUFFER),
    GLMSTOKM("readonly", GLMS_TOKEN_TYPE_SPECIAL_READONLY),
    GLMSTOKM("uniform", GLMS_TOKEN_TYPE_SPECIAL_UNIFORM),
    GLMSTOKM("sampler2D", GLMS_TOKEN_TYPE_SPECIAL_SAMPLER_2D),
    GLMSTOKM("sampler3D", GLMS_TOKEN_TYPE_SPECIAL_SAMPLER_3D),
    GLMSTOKM("samplerCube", GLMS_TOKEN_TYPE_SPECIAL_SAMPLER_CUBE),
    GLMSTOKM("import", GLMS_TOKEN_TYPE_SPECIAL_IMPORT),
    GLMSTOKM("include", GLMS_TOKEN_TYPE_SPECIAL_INCLUDE),
    GLMSTOKM("layout", GLMS_TOKEN_TYPE_SPECIAL_LAYOUT),
    GLMSTOKM("as", GLMS_TOKEN_TYPE_SPECIAL_AS),
    GLMSTOKM("in", GLMS_TOKEN_TYPE_SPECIAL_IN),
    GLMSTOKM("out", GLMS_TOKEN_TYPE_SPECIAL_OUT),
    GLMSTOKM("inout", GLMS_TOKEN_TYPE_SPECIAL_INOUT),
    GLMSTOKM("function", GLMS_TOKEN_TYPE_SPECIAL_FUNCTION),
    GLMSTOKM("return", GLMS_TOKEN_TYPE_SPECIAL_RETURN),
    GLMSTOKM("if", GLMS_TOKEN_TYPE_SPECIAL_IF),
    GLMSTOKM("else", GLMS_TOKEN_TYPE_SPECIAL_ELSE),
    GLMSTOKM("false", GLMS_TOKEN_TYPE_SPECIAL_FALSE),
    GLMSTOKM("true", GLMS_TOKEN_TYPE_SPECIAL_TRUE),
    GLMSTOKM("for", GLMS_TOKEN_TYPE_SPECIAL_FOR),
    GLMSTOKM("switch", GLMS_TOKEN_TYPE_SPECIAL_SWITCH),
    GLMSTOKM("case", GLMS_TOKEN_TYPE_SPECIAL_CASE),
    GLMSTOKM("break", GLMS_TOKEN_TYPE_SPECIAL_BREAK),
    GLMSTOKM("while", GLMS_TOKEN_TYPE_SPECIAL_WHILE),
    GLMSTOKM("string", GLMS_TOKEN_TYPE_SPECIAL_STRING),
    GLMSTOKM("number", GLMS_TOKEN_TYPE_SPECIAL_NUMBER),
    GLMSTOKM("int", GLMS_TOKEN_TYPE_SPECIAL_INT),
    GLMSTOKM("float", GLMS_TOKEN_TYPE_SPECIAL_FLOAT),
    GLMSTOKM("uint64_t", GLMS_TOKEN_TYPE_SPECIAL_UINT64),
    GLMSTOKM("void", GLMS_TOKEN_TYPE_SPECIAL_VOID),
    GLMSTOKM("null", GLMS_TOKEN_TYPE_SPECIAL_NULL),
    GLMSTOKM("array", GLMS_TOKEN_TYPE_SPECIAL_ARRAY),
    GLMSTOKM("object", GLMS_TOKEN_TYPE_SPECIAL_OBJECT),
    GLMSTOKM("struct", GLMS_TOKEN_TYPE_SPECIAL_STRUCT),
    GLMSTOKM("let", GLMS_TOKEN_TYPE_SPECIAL_LET),
    GLMSTOKM("const", GLMS_TOKEN_TYPE_SPECIAL_CONST),
    GLMSTOKM("typedef", GLMS_TOKEN_TYPE_SPECIAL_TYPEDEF),
    GLMSTOKM("vec2", GLMS_TOKEN_TYPE_SPECIAL_VEC2),
    GLMSTOKM("vec3", GLMS_TOKEN_TYPE_SPECIAL_VEC3),
    GLMSTOKM("vec3", GLMS_TOKEN_TYPE_SPECIAL_VEC3),
    GLMSTOKM("mat3", GLMS_TOKEN_TYPE_SPECIAL_MAT3),
    GLMSTOKM("mat4", GLMS_TOKEN_TYPE_SPECIAL_MAT4),
    GLMSTOKM("bool", GLMS_TOKEN_TYPE_SPECIAL_BOOL),
    GLMSTOKM("enum", GLMS_TOKEN_TYPE_SPECIAL_ENUM)};

int glms_lexer_init(GLMSLexer* lexer, const char* source) {
  if (!lexer || !source) return 0;
  if (lexer->initialized) return 1;

  glms_lexer_reset(lexer);
  
  lexer->initialized = true;

  lexer->line = 0;
  lexer->column = 0;
  lexer->source = source;
  lexer->i = 0;
  lexer->length = strlen(lexer->source);
  lexer->c = lexer->source[lexer->i];

  return 1;
}

static char glms_lexer_peek(GLMSLexer* lexer, int i) {
  if ((lexer->i + i) >= lexer->length || (lexer->i + i) < 0) return 0;

  return lexer->source[lexer->i + i];
}

#define GLMS_LEXER_HAS_WHITESPACE                             \
  (lexer->c == ' ' || lexer->c == '\t' || lexer->c == '\n' || \
   lexer->c == 10 || lexer->c == 13)

#define GLMS_LEXER_HAS_COMMENT \
  ((lexer->c == '/' && glms_lexer_peek(lexer, 1) == '/') || lexer->c == '#')

#define GLMS_LEXER_HAS_BLOCK_COMMENT \
  (lexer->c == '/' && glms_lexer_peek(lexer, 1) == '*')

static int glms_lexer_advance(GLMSLexer* lexer) {
  if (lexer->i >= lexer->length) return 0;
  if (lexer->c == 0) return 0;


  if (lexer->c == '\n') {
    lexer->line += 1;
    lexer->column = 0;
  } else {
    lexer->column += 1;
  }

  
  lexer->i += 1;
  lexer->c = lexer->source[lexer->i];

    
  return lexer->c != 0;
}

static int glms_lexer_skip_whitespace(GLMSLexer* lexer) {
  while (lexer->c != 0 && GLMS_LEXER_HAS_WHITESPACE) {
    if (!glms_lexer_advance(lexer)) return 0;
  }

  return lexer->c != 0 && lexer->i < lexer->length;
}

static int glms_lexer_skip_line_comment(GLMSLexer* lexer) {
  while (lexer->c == '/') {
    glms_lexer_advance(lexer);
  }
  while (lexer->c != 0 && lexer->c != '\n' && lexer->c != '\r') {
    if (!glms_lexer_advance(lexer)) return 0;
  }

  return lexer->c != 0 && lexer->i < lexer->length;
}
static int glms_lexer_skip_block_comment(GLMSLexer* lexer) {
  glms_lexer_advance(lexer); // '/'
  while (lexer->c != 0 && !(lexer->c == '*' && glms_lexer_peek(lexer, 1) == '/')) {
    if (!glms_lexer_advance(lexer)) return 0;
  }

  if (lexer->c == '*') {
    glms_lexer_advance(lexer);  // '*'

    if (lexer->c != '/') {
      GLMS_WARNING_RETURN(0, stderr, "Unclosed block-comment.\n");
    }
    glms_lexer_advance(lexer); // '/'
  }

  return lexer->c != 0 && lexer->i < lexer->length;
}

static int glms_lexer_skip_hash_comment(GLMSLexer* lexer) {
  while (lexer->c == '#') {
    glms_lexer_advance(lexer);
  }

  while (lexer->c != 0 && lexer->c != '\n' && lexer->c != '\r') {
    if (!glms_lexer_advance(lexer)) return 0;
  }

  return lexer->c != 0 && lexer->i < lexer->length;
}

static void glms_lexer_parse_special_id(GLMSLexer* lexer, GLMSToken* out) {
  const char* value = glms_string_view_get_value(&out->value);

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

static int glms_lexer_parse_id(GLMSLexer* lexer, GLMSToken* out) {
  out->value.length = 0;
  out->value.ptr = &lexer->source[lexer->i];

  while ((isalnum(lexer->c) || lexer->c == '_') && lexer->c != 0) {
    glms_lexer_advance(lexer);
    out->value.length++;
  }

  out->type = GLMS_TOKEN_TYPE_ID;

  glms_lexer_parse_special_id(lexer, out);

  return 1;
}

static int glms_lexer_parse_string(GLMSLexer* lexer, GLMSToken* out) {
  out->value.length = 0;
  glms_lexer_advance(lexer);
  out->value.ptr = &lexer->source[lexer->i];

  while (lexer->c != '"' && lexer->c != 0) {
    glms_lexer_advance(lexer);
    out->value.length++;

    if (lexer->c == '\\') {
      glms_lexer_advance(lexer);
      glms_lexer_advance(lexer);
      out->value.length += 2;
    }
  }

  glms_lexer_advance(lexer);
  out->type = GLMS_TOKEN_TYPE_STRING;

  return 1;
}

static int glms_lexer_parse_template_string(GLMSLexer* lexer, GLMSToken* out) {
  out->value.length = 0;
  glms_lexer_advance(lexer);
  out->value.ptr = &lexer->source[lexer->i];

  while (lexer->c != '`' && lexer->c != 0) {
    glms_lexer_advance(lexer);
    out->value.length++;

    if (lexer->c == '\\') {
      glms_lexer_advance(lexer);
      glms_lexer_advance(lexer);
      out->value.length += 2;
    }
  }

  glms_lexer_advance(lexer);
  out->type = GLMS_TOKEN_TYPE_TEMPLATE_STRING;

  return 1;
}

static int glms_lexer_parse_number(GLMSLexer* lexer, GLMSToken* out) {
  out->value.length = 0;
  out->value.ptr = &lexer->source[lexer->i];

  out->type = GLMS_TOKEN_TYPE_INT;

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

    out->type = GLMS_TOKEN_TYPE_FLOAT;
  }

  return 1;
}

int glms_lexer_next(GLMSLexer* lexer, GLMSToken* out) {
  if (!lexer || !out) return 0;
  if (!lexer->initialized)
    GLMS_WARNING_RETURN(0, stderr, "Lexer not initialized.\n");
  if (lexer->i >= lexer->length) return 0;
  if (lexer->c == 0) return 0;

  

  out->c = 0;
  out->value.ptr = 0;
  out->value.length = 0;
  out->type = GLMS_TOKEN_TYPE_EOF;

  while (GLMS_LEXER_HAS_COMMENT || GLMS_LEXER_HAS_BLOCK_COMMENT || GLMS_LEXER_HAS_WHITESPACE) {
    while (lexer->c == '#') {
      if (!glms_lexer_skip_hash_comment(lexer)) return 0;
    }
    
    while (GLMS_LEXER_HAS_COMMENT) {
      if (!glms_lexer_skip_line_comment(lexer)) return 0;
    }

    while (GLMS_LEXER_HAS_BLOCK_COMMENT) {
      if (!glms_lexer_skip_block_comment(lexer)) return 0;
    }

    while (GLMS_LEXER_HAS_WHITESPACE) {
      if (!glms_lexer_skip_whitespace(lexer)) return 0;
    } 
  }

  switch (lexer->c) {
    case '!': {
      out->type = GLMS_TOKEN_TYPE_EXCLAM;
    }; break;
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
    case '?': {
      out->type = GLMS_TOKEN_TYPE_QUESTION;
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

      if (glms_lexer_peek(lexer, 1) == '=') {
        out->type = GLMS_TOKEN_TYPE_MUL_EQUALS;
        glms_lexer_advance(lexer);
      }

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

      if (glms_lexer_peek(lexer, 1) == '=') {
        out->type = GLMS_TOKEN_TYPE_DIV_EQUALS;
        glms_lexer_advance(lexer);
      }
    } break;
    case '%': {
      out->type = GLMS_TOKEN_TYPE_PERCENT;
    } break;
    case '|': {
      if (glms_lexer_peek(lexer, 1) == '|') {
        out->type = GLMS_TOKEN_TYPE_PIPE_PIPE;
        glms_lexer_advance(lexer);
      }
    } break;
    case '&': {
      if (glms_lexer_peek(lexer, 1) == '&') {
        out->type = GLMS_TOKEN_TYPE_AND_AND;
        glms_lexer_advance(lexer);
      }
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
      if (lexer->c == '`') {
        return glms_lexer_parse_template_string(lexer, out);
      }
      if (isdigit(lexer->c)) {
        return glms_lexer_parse_number(lexer, out);
      }
      if (isalpha(lexer->c)) {
        return glms_lexer_parse_id(lexer, out);
      }
      GLMS_WARNING_RETURN(0, stderr, "Unexpected token `%c` at %s.\n", lexer->c, glms_lexer_get_position_text(lexer));
    }; break;
  }

  out->c = lexer->c;

  glms_lexer_advance(lexer);

  return lexer->c != 0;
}


int glms_lexer_reset(GLMSLexer *lexer) {
  if (!lexer) return 0;

  lexer->c = 0;
  lexer->i = 0;
  lexer->initialized = false;
  lexer->length = 0;
  lexer->line = 0;
  lexer->column = 0;
  memset(&lexer->linecol[0], 0, GLMS_LEXER_LINE_COLUMN_TEXT_CAP * sizeof(char));

  return 1;
}

const char *glms_lexer_get_position_text(GLMSLexer *lexer) {
  if (!lexer) return 0;
  memset(&lexer->linecol[0], 0, GLMS_LEXER_LINE_COLUMN_TEXT_CAP * sizeof(char));
  snprintf(&lexer->linecol[0], GLMS_LEXER_LINE_COLUMN_TEXT_CAP-1, "%ld:%ld", lexer->line, lexer->column);
  return lexer->linecol;
}
