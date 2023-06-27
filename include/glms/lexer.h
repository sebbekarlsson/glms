#ifndef GLMS_LEXER_H
#define GLMS_LEXER_H
#include <glms/token.h>
#include <stdbool.h>
#include <stdint.h>

#define GLMS_LEXER_LINE_COLUMN_TEXT_CAP 256

typedef struct {
  int64_t i;
  int64_t length;

  int64_t line;
  int64_t column;
  
  char c;
  const char* source;
  bool initialized;

  char linecol[GLMS_LEXER_LINE_COLUMN_TEXT_CAP];
} GLMSLexer;

int glms_lexer_init(GLMSLexer* lexer, const char* source);

int glms_lexer_next(GLMSLexer *lexer, GLMSToken *out);

int glms_lexer_reset(GLMSLexer *lexer);

const char* glms_lexer_get_position_text(GLMSLexer* lexer);

#endif
