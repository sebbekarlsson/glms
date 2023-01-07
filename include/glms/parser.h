#ifndef GLMS_PARSER_H
#define GLMS_PARSER_H
#include <glms/ast.h>
#include <glms/lexer.h>
#include <memo/memo.h>
#include <stdbool.h>

struct GLMS_ENV_STRUCT;

typedef struct {
  struct GLMS_ENV_STRUCT* env;
  bool error;
  bool finished;
  GLMSToken token;
  bool initialized;

  HashyMap symbols;
} GLMSParser;

int glms_parser_init(GLMSParser* parser, struct GLMS_ENV_STRUCT* env);

int glms_parser_eat(GLMSParser* parser, GLMSTokenType token_type);

GLMSAST* glms_parser_parse(GLMSParser* parser);

GLMSAST* glms_parser_parse_id(GLMSParser* parser, bool skip_next);

GLMSAST* glms_parser_parse_number(GLMSParser* parser);
GLMSAST* glms_parser_parse_int(GLMSParser* parser);
GLMSAST* glms_parser_parse_float(GLMSParser* parser);
GLMSAST* glms_parser_parse_null(GLMSParser* parser);
GLMSAST* glms_parser_parse_bool(GLMSParser* parser);
GLMSAST* glms_parser_parse_string(GLMSParser* parser);
GLMSAST* glms_parser_parse_template_string(GLMSParser* parser);
GLMSAST* glms_parser_parse_array(GLMSParser* parser);
GLMSAST* glms_parser_parse_object(GLMSParser* parser);

GLMSAST* glms_parser_parse_expr(GLMSParser* parser);

GLMSAST* glms_parser_parse_compound(GLMSParser* parser, bool skip_brace);

GLMSAST* glms_parser_parse_term(GLMSParser* parser);
GLMSAST* glms_parser_parse_factor(GLMSParser* parser);
GLMSAST* glms_parser_parse_call(GLMSParser* parser, GLMSAST* left);
GLMSAST* glms_parser_parse_arrow_function(GLMSParser* parser);
GLMSAST* glms_parser_parse_function(GLMSParser* parser);
GLMSAST* glms_parser_parse_block(GLMSParser* parser);
GLMSAST* glms_parser_parse_for(GLMSParser* parser);

GLMSAST* glms_parser_parse_binop(GLMSParser* parser, GLMSAST* left);
GLMSAST* glms_parser_parse_struct(GLMSParser* parser);
GLMSAST* glms_parser_parse_enum(GLMSParser* parser);
GLMSAST* glms_parser_parse_unop(GLMSParser* parser);
GLMSAST* glms_parser_parse_typedef(GLMSParser* parser);
GLMSAST* glms_parser_parse_import(GLMSParser* parser);

GLMSAST* glms_parser_lookup(GLMSParser* parser, const char* key);

bool glms_parser_peek_check_arrow_function(GLMSParser* parser);

#endif
