#ifndef GLMS_LEXER_H
#define GLMS_LEXER_H
#include <glms/token.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  int64_t i;
  int64_t length;
  char c;
  const char* source;
  bool initialized;
} GLMSLexer;

int glms_lexer_init(GLMSLexer* lexer, const char* source);

int glms_lexer_next(GLMSLexer *lexer, GLMSToken *out);

int glms_lexer_reset(GLMSLexer* lexer);

#endif
