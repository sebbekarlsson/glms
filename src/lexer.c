#include <jscript/lexer.h>
#include <jscript/macros.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>


int jscript_lexer_init(
  JSCRIPTLexer* lexer,
  const char* source
) {
  if (!lexer || !source) return 0;
  if (lexer->initialized) return 1;
  lexer->initialized = true;

  lexer->source = source;
  lexer->i = 0;
  lexer->length = strlen(lexer->source);
  lexer->c = lexer->source[lexer->i];

  return 1;
}


static char jscript_lexer_peek(JSCRIPTLexer* lexer, int i ) {
  if ((lexer->i+i) >= lexer->length) return 0;

  return lexer->source[lexer->i+i];
}

#define JSCRIPT_LEXER_HAS_WHITESPACE (lexer->c == ' ' || lexer->c == '\t' || lexer->c == '\n' || lexer->c == 10 || lexer->c == 13)

static int jscript_lexer_advance(JSCRIPTLexer* lexer) {
  if (lexer->i >= lexer->length) return 0;
  if (lexer->c == 0) return 0;
  lexer->i += 1;
  lexer->c = lexer->source[lexer->i];
  return lexer->c != 0;
}

static int jscript_lexer_skip_whitespace(JSCRIPTLexer* lexer) {
  while (lexer->c != 0 && JSCRIPT_LEXER_HAS_WHITESPACE) {
    if (!jscript_lexer_advance(lexer)) return 0;
  }

  return lexer->c != 0 && lexer->i < lexer->length;
}

static int jscript_lexer_parse_id(JSCRIPTLexer* lexer, JSCRIPTToken* out) {
  out->value.length = 0;
  out->value.ptr = &lexer->source[lexer->i];


  while (isalnum(lexer->c) && lexer->c != 0) {
    jscript_lexer_advance(lexer);
    out->value.length++;
  }

  out->type = JSCRIPT_TOKEN_TYPE_ID;

  return 1;
}

static int jscript_lexer_parse_number(JSCRIPTLexer* lexer, JSCRIPTToken* out) {
  out->value.length = 0;
  out->value.ptr = &lexer->source[lexer->i];


  while (isdigit(lexer->c) && lexer->c != 0) {
    jscript_lexer_advance(lexer);
    out->value.length++;
  }

  if (lexer->c == '.') {
    out->value.length++;
    jscript_lexer_advance(lexer);

    while (isdigit(lexer->c) && lexer->c != 0) {
      jscript_lexer_advance(lexer);
      out->value.length++;
    }
  }

  out->type = JSCRIPT_TOKEN_TYPE_NUMBER;

  return 1;
}

int jscript_lexer_next(JSCRIPTLexer* lexer, JSCRIPTToken* out) {
  if (!lexer || !out) return 0;
  if (!lexer->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "Lexer not initialized.\n");
  if (lexer->i >= lexer->length) return 0;
  if (lexer->c == 0) return 0;

  out->c = 0;
  out->value.ptr = 0;
  out->value.length = 0;
  out->type=  JSCRIPT_TOKEN_TYPE_NONE;

  while (JSCRIPT_LEXER_HAS_WHITESPACE) {
    if (!jscript_lexer_skip_whitespace(lexer)) return 0;
  }


  switch (lexer->c) {
    case '{': {
      out->type = JSCRIPT_TOKEN_TYPE_LBRACE;
    } break;
    case '}': {
      out->type = JSCRIPT_TOKEN_TYPE_RBRACE;
    } break;
    case '[': {
      out->type = JSCRIPT_TOKEN_TYPE_LBRACKET;
    } break;
    case ']': {
      out->type = JSCRIPT_TOKEN_TYPE_RBRACKET;
    } break;
    case '(': {
      out->type = JSCRIPT_TOKEN_TYPE_LPAREN;
    } break;
    case ')': {
      out->type = JSCRIPT_TOKEN_TYPE_RPAREN;
    } break;
    case ';': {
      out->type = JSCRIPT_TOKEN_TYPE_SEMI;
    } break;
    case ',': {
      out->type = JSCRIPT_TOKEN_TYPE_COMMA;
    } break;
    case ':': {
      out->type = JSCRIPT_TOKEN_TYPE_COLON;
    } break;
    case '+': {
      out->type = JSCRIPT_TOKEN_TYPE_ADD;
    } break;
    case '*': {
      out->type = JSCRIPT_TOKEN_TYPE_MUL;
    } break;
    case '-': {
      out->type = JSCRIPT_TOKEN_TYPE_SUB;
    } break;
    case '/': {
      out->type = JSCRIPT_TOKEN_TYPE_DIV;
    } break;
    case '%': {
      out->type = JSCRIPT_TOKEN_TYPE_PERCENT;
    } break;
    case '=': {
      out->type = JSCRIPT_TOKEN_TYPE_EQUALS;

      if (jscript_lexer_peek(lexer, 1) == '=') {
        out->type = JSCRIPT_TOKEN_TYPE_EQUALS_EQUALS;
        jscript_lexer_advance(lexer);
      }
    } break;
    case '>': {
      out->type = JSCRIPT_TOKEN_TYPE_GT;

      if (jscript_lexer_peek(lexer, 1) == '=') {
        out->type = JSCRIPT_TOKEN_TYPE_GTE;
        jscript_lexer_advance(lexer);
      }
    } break;
    case '<': {
      out->type = JSCRIPT_TOKEN_TYPE_LT;

      if (jscript_lexer_peek(lexer, 1) == '=') {
        out->type = JSCRIPT_TOKEN_TYPE_LTE;
        jscript_lexer_advance(lexer);
      }
    } break;
    default: {
      if (isdigit(lexer->c)) {
        return jscript_lexer_parse_number(lexer, out);
      }
      if (isalpha(lexer->c)) {
        return jscript_lexer_parse_id(lexer, out);
      }
      JSCRIPT_WARNING_RETURN(0, stderr, "Unexpected token `%c`.\n", lexer->c);
    }; break;
  }

  out->c = lexer->c;

  jscript_lexer_advance(lexer);

  return 1;
}
