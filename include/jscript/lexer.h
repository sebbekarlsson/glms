#ifndef JSCRIPT_LEXER_H
#define JSCRIPT_LEXER_H
#include <jscript/token.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
  int64_t i;
  int64_t length;
  char c;
  const char* source;
  bool initialized;
} JSCRIPTLexer;


int jscript_lexer_init(
  JSCRIPTLexer* lexer,
  const char* source
);


#define JSCRIPT_LEXER_SPECIAL_SYMBOL_FUNCTION "function"
#define JSCRIPT_LEXER_SPECIAL_SYMBOL_RETURN "return"

int jscript_lexer_next(JSCRIPTLexer* lexer, JSCRIPTToken* out);

#endif
