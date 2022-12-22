#ifndef JSCRIPT_PARSER_H
#define JSCRIPT_PARSER_H
#include <jscript/lexer.h>
#include <jscript/ast.h>
#include <memo/memo.h>
#include <stdbool.h>

struct JSCRIPT_ENV_STRUCT;

typedef struct {
  struct JSCRIPT_ENV_STRUCT* env;
  bool error;
  JSCRIPTToken token;
  bool initialized;
} JSCRIPTParser;


int jscript_parser_init(
  JSCRIPTParser* parser,
  struct JSCRIPT_ENV_STRUCT* env
);

int jscript_parser_eat(JSCRIPTParser* parser, JSCRIPTTokenType token_type);

JSCRIPTAST* jscript_parser_parse(JSCRIPTParser* parser);

JSCRIPTAST* jscript_parser_parse_id(JSCRIPTParser* parser, bool skip_next);

JSCRIPTAST* jscript_parser_parse_number(JSCRIPTParser* parser);
JSCRIPTAST* jscript_parser_parse_string(JSCRIPTParser* parser);

JSCRIPTAST* jscript_parser_parse_expr(JSCRIPTParser* parser);


JSCRIPTAST* jscript_parser_parse_compound(JSCRIPTParser* parser, bool skip_brace);

JSCRIPTAST* jscript_parser_parse_term(JSCRIPTParser* parser);
JSCRIPTAST* jscript_parser_parse_factor(JSCRIPTParser* parser);
JSCRIPTAST* jscript_parser_parse_call(JSCRIPTParser* parser, JSCRIPTAST* left);
JSCRIPTAST* jscript_parser_parse_arrow_function(JSCRIPTParser* parser);
JSCRIPTAST* jscript_parser_parse_function(JSCRIPTParser* parser);


JSCRIPTAST* jscript_parser_parse_binop(JSCRIPTParser* parser, JSCRIPTAST* left);
JSCRIPTAST* jscript_parser_parse_unop(JSCRIPTParser* parser);

bool jscript_parser_peek_check_arrow_function(JSCRIPTParser* parser);



#endif
