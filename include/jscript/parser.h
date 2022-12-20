#ifndef JSCRIPT_PARSER_H
#define JSCRIPT_PARSER_H
#include <jscript/lexer.h>
#include <jscript/ast.h>
#include <memo/memo.h>
#include <stdbool.h>

struct JSCRIPT_ENV_STRUCT;

typedef struct {
  struct JSCRIPT_ENV_STRUCT* env;
  JSCRIPTToken token;
  bool initialized;
} JSCRIPTParser;


int jscript_parser_init(
  JSCRIPTParser* parser,
  struct JSCRIPT_ENV_STRUCT* env
);

int jscript_parser_eat(JSCRIPTParser* parser, JSCRIPTTokenType token_type);

JSCRIPTAST* jscript_parser_parse(JSCRIPTParser* parser);

JSCRIPTAST* jscript_parser_parse_id(JSCRIPTParser* parser);

JSCRIPTAST* jscript_parser_parse_number(JSCRIPTParser* parser);

JSCRIPTAST* jscript_parser_parse_expr(JSCRIPTParser* parser);

JSCRIPTAST* jscript_parser_parse_term(JSCRIPTParser* parser);
JSCRIPTAST* jscript_parser_parse_factor(JSCRIPTParser* parser);


JSCRIPTAST* jscript_parser_parse_binop(JSCRIPTParser* parser, JSCRIPTAST* left);



#endif
