#include <jscript/env.h>
#include <jscript/macros.h>
#include <jscript/parser.h>

int jscript_parser_init(JSCRIPTParser *parser, JSCRIPTEnv *env) {
  if (!parser || !env)
    return 0;
  if (parser->initialized)
    return 1;

  parser->initialized = true;
  parser->env = env;
  jscript_lexer_next(&parser->env->lexer, &parser->token);

  return 1;
}

int jscript_parser_eat(JSCRIPTParser *parser, JSCRIPTTokenType token_type) {
  if (!parser)
    return 0;
  if (!parser->initialized)
    JSCRIPT_WARNING_RETURN(0, stderr, "parser not initialized.\n");

  if (parser->token.type != token_type) {
    JSCRIPT_WARNING_RETURN(0, stderr, "Unexpected token `%d`\n",
                           parser->token.type);
  }

  if (!jscript_lexer_next(&parser->env->lexer, &parser->token))
    return 0;

  return 1;
}

static JSCRIPTAST *jscript_parser_parse_noop(JSCRIPTParser *parser) {
  JSCRIPTAST *ast = jscript_env_new_ast(parser->env, JSCRIPT_AST_TYPE_NOOP);
  return ast;
}

static JSCRIPTAST *jscript_parser_parse_eof(JSCRIPTParser *parser) {
  JSCRIPTAST *ast = jscript_env_new_ast(parser->env, JSCRIPT_AST_TYPE_EOF);
  return ast;
}

JSCRIPTAST *jscript_parser_parse_id(JSCRIPTParser *parser) {
  JSCRIPTAST *ast = jscript_env_new_ast(parser->env, JSCRIPT_AST_TYPE_ID);
  ast->as.id.value = parser->token.value;
  jscript_parser_eat(parser, JSCRIPT_TOKEN_TYPE_ID);

  JSCRIPTTokenType next_type = parser->token.type;

  if (next_type == JSCRIPT_TOKEN_TYPE_ID) {
    jscript_parser_eat(parser, JSCRIPT_TOKEN_TYPE_ID);
    JSCRIPTAST *next_ast =
        jscript_env_new_ast(parser->env, JSCRIPT_AST_TYPE_ID);
    next_ast->as.id.flag = ast;
    next_ast->as.id.value = parser->token.value;

    ast = next_ast;
  }

  return ast;
}

JSCRIPTAST *jscript_parser_parse_number(JSCRIPTParser *parser) {
  JSCRIPTAST *ast = jscript_env_new_ast(parser->env, JSCRIPT_AST_TYPE_NUMBER);
  ast->as.number.value =
      atof(jscript_string_view_get_value(&parser->token.value));
  jscript_parser_eat(parser, JSCRIPT_TOKEN_TYPE_NUMBER);
  return ast;
}

JSCRIPTAST *jscript_parser_parse_binop(JSCRIPTParser *parser,
                                       JSCRIPTAST *left) {
  JSCRIPTAST *ast = jscript_env_new_ast(parser->env, JSCRIPT_AST_TYPE_BINOP);
  ast->as.binop.left = left;
  ast->as.binop.op = parser->token.type;
  jscript_parser_eat(parser, parser->token.type);
  ast->as.binop.right = jscript_parser_parse_term(parser);
  return ast;
}

JSCRIPTAST *jscript_parser_parse_factor(JSCRIPTParser *parser) {
  if (parser->token.type == JSCRIPT_TOKEN_TYPE_LPAREN) {
    jscript_parser_eat(parser, parser->token.type);
    JSCRIPTAST* next = jscript_parser_parse_expr(parser);
    jscript_parser_eat(parser, JSCRIPT_TOKEN_TYPE_RPAREN);
    return next;
  }
  switch (parser->token.type) {
  case JSCRIPT_TOKEN_TYPE_ID: {
    return jscript_parser_parse_id(parser);
  }; break;
  case JSCRIPT_TOKEN_TYPE_NUMBER: {
    return jscript_parser_parse_number(parser);
  }; break;
  case JSCRIPT_TOKEN_TYPE_EOF: {
    return jscript_parser_parse_eof(parser);
  }; break;
  default: {
    return jscript_parser_parse_noop(parser);
  }; break;
  }
  return jscript_parser_parse_noop(parser);
}

JSCRIPTAST *jscript_parser_parse_term(JSCRIPTParser *parser) {
  JSCRIPTAST *left = jscript_parser_parse_factor(parser);

  while (
    parser->token.type == JSCRIPT_TOKEN_TYPE_MUL ||
    parser->token.type == JSCRIPT_TOKEN_TYPE_DIV
  ) {
    JSCRIPTAST* binop = jscript_env_new_ast(parser->env, JSCRIPT_AST_TYPE_BINOP);
    binop->as.binop.left = left;
    binop->as.binop.op = parser->token.type;
    jscript_parser_eat(parser, parser->token.type);
    binop->as.binop.right = jscript_parser_parse_expr(parser);
    left = binop;
  }


  return left;
}

JSCRIPTAST *jscript_parser_parse_expr(JSCRIPTParser *parser) {
  JSCRIPTAST *left = jscript_parser_parse_term(parser);

  while (
    parser->token.type == JSCRIPT_TOKEN_TYPE_ADD ||
    parser->token.type == JSCRIPT_TOKEN_TYPE_SUB
  ) {
    JSCRIPTAST* binop = jscript_env_new_ast(parser->env, JSCRIPT_AST_TYPE_BINOP);
    binop->as.binop.left = left;
    binop->as.binop.op = parser->token.type;
    jscript_parser_eat(parser, parser->token.type);
    binop->as.binop.right = jscript_parser_parse_term(parser);
    left = binop;
  }

  while (parser->token.type == JSCRIPT_TOKEN_TYPE_EQUALS) {
    JSCRIPTAST* binop = jscript_env_new_ast(parser->env, JSCRIPT_AST_TYPE_BINOP);
    binop->as.binop.left = left;
    binop->as.binop.op = parser->token.type;
    jscript_parser_eat(parser, parser->token.type);
    binop->as.binop.right = jscript_parser_parse_expr(parser);
    left = binop;
  }

  return left;
}

JSCRIPTAST *jscript_parser_parse(JSCRIPTParser *parser) {
  if (!parser)
    return 0;
  if (!parser->initialized)
    JSCRIPT_WARNING_RETURN(0, stderr, "parser not initialized.\n");

  JSCRIPTAST *ast = jscript_env_new_ast(parser->env, JSCRIPT_AST_TYPE_COMPOUND);

  while (parser->token.type != JSCRIPT_TOKEN_TYPE_EOF) {

    while (parser->token.type != JSCRIPT_TOKEN_TYPE_SEMI) {
      JSCRIPTAST *child = jscript_parser_parse_expr(parser);

      if (!child)
        break;

      jscript_ast_push(ast, child);

      if (child->type == JSCRIPT_AST_TYPE_EOF)
        break;
    }

    while (parser->token.type == JSCRIPT_TOKEN_TYPE_SEMI) {
      jscript_parser_eat(parser, parser->token.type);
    }
  }

  return jscript_parser_parse_noop(parser);
}
