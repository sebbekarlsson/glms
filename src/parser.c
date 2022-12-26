#include <glms/env.h>
#include <glms/macros.h>
#include <glms/parser.h>

int glms_parser_init(GLMSParser *parser, GLMSEnv *env) {
  if (!parser || !env)
    return 0;
  if (parser->initialized)
    return 1;

  parser->initialized = true;
  parser->env = env;
  glms_lexer_next(&parser->env->lexer, &parser->token);
  hashy_map_init(&parser->symbols, 256);

  const char *tokname = glms_string_view_get_value(&parser->token.value);
  GLMSAST *known = glms_parser_lookup(parser, tokname);

  if (known && (known->type == GLMS_AST_TYPE_TYPEDEF ||
                known->type == GLMS_AST_TYPE_STRUCT)) {

    parser->token.type = GLMS_TOKEN_TYPE_SPECIAL_USER_TYPE;
  }

  return 1;
}

int glms_parser_eat(GLMSParser *parser, GLMSTokenType token_type) {
  if (!parser)
    return 0;
  if (!parser->initialized)
    GLMS_WARNING_RETURN(0, stderr, "parser not initialized.\n");

  if (parser->token.type != token_type) {
    GLMS_WARNING_RETURN(0, stderr, "Unexpected token `%s`\n",
                        GLMS_TOKEN_TYPE_STR[parser->token.type]);
  }

  if (!glms_lexer_next(&parser->env->lexer, &parser->token))
    return 0;

  const char *tokname = glms_string_view_get_value(&parser->token.value);
  GLMSAST *known = glms_parser_lookup(parser, tokname);

  if (known && (known->type == GLMS_AST_TYPE_TYPEDEF ||
                known->type == GLMS_AST_TYPE_STRUCT)) {

    parser->token.type = GLMS_TOKEN_TYPE_SPECIAL_USER_TYPE;
  }

  return 1;
}

static GLMSAST *glms_parser_error(GLMSParser *parser) {
  GLMS_WARNING(stderr, "Unexpected token `%s`\n",
               GLMS_TOKEN_TYPE_STR[parser->token.type]);
  parser->error = true;

  return glms_env_new_ast(parser->env, GLMS_AST_TYPE_EOF);
}

static GLMSAST *glms_parser_parse_noop(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_NOOP);
  return ast;
}

static GLMSAST *glms_parser_parse_eof(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_EOF);
  return ast;
}

GLMSAST *glms_parser_parse_id(GLMSParser *parser, bool skip_next) {
  GLMSASTList *flags = 0;

  if (glms_token_type_is_flag(parser->token.type)) {
    flags = NEW(GLMSASTList);
    glms_GLMSAST_list_init(flags);

    while (glms_token_type_is_flag(parser->token.type)) {
      GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_ID);
      ast->as.id.op = parser->token.type;
      ast->as.id.value = parser->token.value;
      glms_parser_eat(parser, parser->token.type);
      glms_GLMSAST_list_push(flags, ast);
    }
  }

  GLMSAST *id_ast = 0;

  if (parser->token.type == GLMS_TOKEN_TYPE_ID) {
    id_ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_ID);
    id_ast->as.id.value = parser->token.value;
    id_ast->as.id.heap = 0;
    id_ast->flags = flags;
    glms_parser_eat(parser, parser->token.type);
  } else if (flags && flags->length == 1) {
    id_ast = flags->items[0];
    glms_GLMSAST_list_clear(flags);
    free(flags);
    flags = 0;
  } else {
    return glms_parser_error(parser);
  }

  if (skip_next)
    return id_ast;

  GLMSTokenType next_type = parser->token.type;

  switch (next_type) {
  case GLMS_TOKEN_TYPE_LPAREN: {
    return glms_parser_parse_call(parser, id_ast);
  }; break;
  default: {
  }; break;
  }

  return id_ast;
}

GLMSAST *glms_parser_parse_number(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_NUMBER);
  ast->as.number.value = atof(glms_string_view_get_value(&parser->token.value));
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_NUMBER);
  return ast;
}

GLMSAST *glms_parser_parse_bool(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_NUMBER);
  ast->as.number.value =
      parser->token.type == GLMS_TOKEN_TYPE_SPECIAL_FALSE ? 0 : 1;
  glms_parser_eat(parser, parser->token.type);
  return ast;
}

GLMSAST *glms_parser_parse_string(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_STRING);
  ast->as.string.value = parser->token.value;
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_STRING);
  return ast;
}

GLMSAST *glms_parser_parse_array(GLMSParser *parser) {
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LBRACKET);

  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_ARRAY);

  GLMSAST *arg = glms_parser_parse_expr(parser);
  glms_ast_push(ast, arg);

  while (parser->token.type == GLMS_TOKEN_TYPE_COMMA) {
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_COMMA);
    GLMSAST *arg = glms_parser_parse_expr(parser);
    glms_ast_push(ast, arg);
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_RBRACKET);

  return ast;
}

static GLMSAST *glms_parser_parse_kv(GLMSParser *parser, const char **key) {
  GLMSAST *ast_key = glms_parser_parse_factor(parser);
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_COLON);
  GLMSAST *ast_value = glms_parser_parse_expr(parser);
  const char *k = glms_ast_get_string_value(ast_key);

  if (!k)
    return 0;

  *key = k;

  return ast_value;
}

GLMSAST *glms_parser_parse_object(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_OBJECT);

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LBRACE);

  if (parser->token.type != GLMS_TOKEN_TYPE_RBRACE) {
    const char *key = 0;
    GLMSAST *child = glms_parser_parse_kv(parser, &key);

    if (child && key != 0) {
      glms_ast_object_set_property(ast, key, child);
    }
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_COMMA) {
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_COMMA);

    const char *key = 0;
    GLMSAST *child = glms_parser_parse_kv(parser, &key);

    if (child && key != 0) {
      glms_ast_object_set_property(ast, key, child);
    }
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_RBRACE);

  return ast;
}

GLMSAST *glms_parser_parse_binop(GLMSParser *parser, GLMSAST *left) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BINOP);
  ast->as.binop.left = left;
  ast->as.binop.op = parser->token.type;
  glms_parser_eat(parser, parser->token.type);
  ast->as.binop.right = glms_parser_parse_term(parser);
  return ast;
}

GLMSAST *glms_parser_parse_struct(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_STRUCT);
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_SPECIAL_STRUCT);

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LBRACE);

  if (parser->token.type != GLMS_TOKEN_TYPE_RBRACE) {
    GLMSAST *field = glms_parser_parse_expr(parser);
    const char *key = glms_ast_get_string_value(field);
    if (!key) {
      GLMS_WARNING(stderr, "key == null.\n");
    }
    glms_ast_object_set_property(ast, key, field);
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_SEMI) {
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_SEMI);

    if (parser->token.type == GLMS_TOKEN_TYPE_RBRACE)
      break;

    GLMSAST *field = glms_parser_parse_expr(parser);
    const char *key = glms_ast_get_string_value(field);
    if (!key) {
      GLMS_WARNING(stderr, "key == null.\n");
      continue;
    }
    glms_ast_object_set_property(ast, key, field);
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_RBRACE);

  return ast;
}

GLMSAST *glms_parser_parse_unop(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_UNOP);
  ast->as.unop.op = parser->token.type;
  glms_parser_eat(parser, parser->token.type);
  ast->as.unop.right = glms_parser_parse_term(parser);
  return ast;
}

GLMSAST *glms_parser_parse_factor(GLMSParser *parser) {
  if (parser->token.type == GLMS_TOKEN_TYPE_SUB ||
      parser->token.type == GLMS_TOKEN_TYPE_ADD) {
    return glms_parser_parse_unop(parser);
  }

  if (parser->token.type == GLMS_TOKEN_TYPE_LPAREN) {

    if (glms_parser_peek_check_arrow_function(parser)) {
      return glms_parser_parse_arrow_function(parser);
    }
    GLMSAST *next = 0;
    glms_parser_eat(parser, parser->token.type);

    if (parser->token.type != GLMS_TOKEN_TYPE_RPAREN) {
      next = glms_parser_parse_expr(parser);
    }

    glms_parser_eat(parser, GLMS_TOKEN_TYPE_RPAREN);

    return next ? next : glms_parser_error(parser);
  }
  switch (parser->token.type) {
  case GLMS_TOKEN_TYPE_LBRACKET: {
    return glms_parser_parse_array(parser);
  }; break;
  case GLMS_TOKEN_TYPE_LBRACE: {
    return glms_parser_parse_object(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_LET:
  case GLMS_TOKEN_TYPE_SPECIAL_CONST:
  case GLMS_TOKEN_TYPE_SPECIAL_STRING:
  case GLMS_TOKEN_TYPE_SPECIAL_NUMBER:
  case GLMS_TOKEN_TYPE_SPECIAL_USER_TYPE:
  case GLMS_TOKEN_TYPE_ID: {
    return glms_parser_parse_id(parser, false);
  }; break;
  case GLMS_TOKEN_TYPE_STRING: {
    return glms_parser_parse_string(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_FALSE:
  case GLMS_TOKEN_TYPE_SPECIAL_TRUE: {
    return glms_parser_parse_bool(parser);
  }; break;
  case GLMS_TOKEN_TYPE_NUMBER: {
    return glms_parser_parse_number(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_TYPEDEF: {
    return glms_parser_parse_typedef(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_STRUCT: {
    return glms_parser_parse_struct(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_FUNCTION: {
    return glms_parser_parse_function(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_RETURN: {
    return glms_parser_parse_unop(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_WHILE:
  case GLMS_TOKEN_TYPE_SPECIAL_IF: {
    return glms_parser_parse_block(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_FOR: {
    return glms_parser_parse_for(parser);
  }; break;
  case GLMS_TOKEN_TYPE_EOF: {
    return glms_parser_parse_eof(parser);
  }; break;
  default: {
    return glms_parser_error(parser);
  }; break;
  }
  return glms_parser_error(parser);
}

GLMSAST *glms_parser_parse_term(GLMSParser *parser) {
  GLMSAST *left = glms_parser_parse_factor(parser);

  while (parser->token.type == GLMS_TOKEN_TYPE_ADD_ADD ||
         parser->token.type == GLMS_TOKEN_TYPE_SUB_SUB) {
    GLMSAST *unop = glms_env_new_ast(parser->env, GLMS_AST_TYPE_UNOP);
    unop->as.unop.left = left;
    unop->as.unop.op = parser->token.type;
    glms_parser_eat(parser, parser->token.type);
    left = unop;
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_MUL ||
         parser->token.type == GLMS_TOKEN_TYPE_DIV) {
    GLMSAST *binop = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BINOP);
    binop->as.binop.left = left;
    binop->as.binop.op = parser->token.type;
    glms_parser_eat(parser, parser->token.type);
    binop->as.binop.right = glms_parser_parse_expr(parser);
    left = binop;
  }
  return left;
}

GLMSAST *glms_parser_parse_expr(GLMSParser *parser) {
  GLMSAST *left = glms_parser_parse_term(parser);

  while (parser->token.type == GLMS_TOKEN_TYPE_LBRACKET) {
    GLMSAST *access = glms_env_new_ast(parser->env, GLMS_AST_TYPE_ACCESS);
    access->as.access.left = left;

    access->as.access.right = glms_parser_parse_expr(parser);
    left = access;
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_ADD ||
         parser->token.type == GLMS_TOKEN_TYPE_SUB ||
         parser->token.type == GLMS_TOKEN_TYPE_ADD_EQUALS ||
         parser->token.type == GLMS_TOKEN_TYPE_SUB_EQUALS) {
    GLMSAST *binop = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BINOP);
    binop->as.binop.left = left;
    binop->as.binop.op = parser->token.type;
    glms_parser_eat(parser, parser->token.type);
    binop->as.binop.right = glms_parser_parse_term(parser);
    left = binop;
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_GT ||
         parser->token.type == GLMS_TOKEN_TYPE_LT ||
         parser->token.type == GLMS_TOKEN_TYPE_GTE ||
         parser->token.type == GLMS_TOKEN_TYPE_LTE ||
         parser->token.type == GLMS_TOKEN_TYPE_EQUALS_EQUALS) {
    GLMSAST *binop = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BINOP);
    binop->as.binop.left = left;
    binop->as.binop.op = parser->token.type;
    glms_parser_eat(parser, parser->token.type);
    binop->as.binop.right = glms_parser_parse_expr(parser);
    left = binop;
  }

  while (left && parser->token.type == GLMS_TOKEN_TYPE_DOT) {
    GLMSAST *access = glms_env_new_ast(parser->env, GLMS_AST_TYPE_ACCESS);
    access->as.access.left = left;

    if (parser->token.type == GLMS_TOKEN_TYPE_DOT) {
      glms_parser_eat(parser, parser->token.type);
    }

    access->as.access.right = glms_parser_parse_term(parser);
    left = access;
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_EQUALS) {
    GLMSAST *binop = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BINOP);
    binop->as.binop.left = left;
    binop->as.binop.op = parser->token.type;
    glms_parser_eat(parser, parser->token.type);
    binop->as.binop.right = glms_parser_parse_expr(parser);
    left = binop;
  }

  return left;
}

GLMSAST *glms_parser_parse_call(GLMSParser *parser, GLMSAST *left) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_CALL);
  ast->as.call.left = left;

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LPAREN);

  if (parser->token.type != GLMS_TOKEN_TYPE_RPAREN) {
    GLMSAST *arg = glms_parser_parse_expr(parser);
    glms_ast_push(ast, arg);
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_COMMA) {
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_COMMA);
    GLMSAST *arg = glms_parser_parse_expr(parser);
    glms_ast_push(ast, arg);
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_RPAREN);

  return ast;
}

GLMSAST *glms_parser_parse_arrow_function(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_FUNC);

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LPAREN);

  if (parser->token.type != GLMS_TOKEN_TYPE_RPAREN) {
    GLMSAST *arg = glms_parser_parse_expr(parser);
    glms_ast_push(ast, arg);
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_COMMA) {
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_COMMA);
    GLMSAST *arg = glms_parser_parse_expr(parser);
    glms_ast_push(ast, arg);
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_RPAREN);

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_EQUALS);
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_GT);

  if (parser->token.type == GLMS_TOKEN_TYPE_LBRACE) {
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_LBRACE);

    if (parser->token.type != GLMS_TOKEN_TYPE_RBRACE) {
      ast->as.func.body = glms_parser_parse_compound(parser, true);
    }

    glms_parser_eat(parser, GLMS_TOKEN_TYPE_RBRACE);
  } else {
    ast->as.func.body = glms_parser_parse_expr(parser);
  }

  return ast;
}

GLMSAST *glms_parser_parse_function(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_FUNC);
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_SPECIAL_FUNCTION);

  if (parser->token.type == GLMS_TOKEN_TYPE_ID) {
    ast->as.func.id = glms_parser_parse_id(parser, true);
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LPAREN);

  if (parser->token.type != GLMS_TOKEN_TYPE_RPAREN) {
    GLMSAST *arg = glms_parser_parse_expr(parser);
    glms_ast_push(ast, arg);
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_COMMA) {
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_COMMA);
    GLMSAST *arg = glms_parser_parse_expr(parser);
    glms_ast_push(ast, arg);
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_RPAREN);

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LBRACE);

  if (parser->token.type != GLMS_TOKEN_TYPE_RBRACE) {
    ast->as.func.body = glms_parser_parse_compound(parser, true);
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_RBRACE);

  return ast;
}

GLMSAST *glms_parser_parse_block(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BLOCK);
  ast->as.block.op = parser->token.type;
  glms_parser_eat(parser, parser->token.type);

  if (parser->token.type == GLMS_TOKEN_TYPE_LPAREN) {
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_LPAREN);
    ast->as.block.expr = glms_parser_parse_expr(parser);
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_RPAREN);
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LBRACE);

  if (parser->token.type != GLMS_TOKEN_TYPE_RBRACE) {
    ast->as.block.body = glms_parser_parse_compound(parser, true);
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_RBRACE);

  if (parser->token.type == GLMS_TOKEN_TYPE_SPECIAL_ELSE) {
    ast->as.block.next = glms_parser_parse_block(parser);
  }

  return ast;
}

GLMSAST *glms_parser_parse_for(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_FOR);
  glms_parser_eat(parser, parser->token.type);

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LPAREN);

  if (parser->token.type != GLMS_TOKEN_TYPE_RPAREN) {
    GLMSAST *arg = glms_parser_parse_expr(parser);
    glms_ast_push(ast, arg);

    while (parser->token.type == GLMS_TOKEN_TYPE_SEMI) {
      glms_parser_eat(parser, GLMS_TOKEN_TYPE_SEMI);
      GLMSAST *arg = glms_parser_parse_expr(parser);
      glms_ast_push(ast, arg);
    }
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_RPAREN);

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LBRACE);

  if (parser->token.type != GLMS_TOKEN_TYPE_RBRACE) {
    ast->as.forloop.body = glms_parser_parse_compound(parser, true);
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_RBRACE);

  return ast;
}

GLMSAST *glms_parser_parse_typedef(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_TYPEDEF);
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_SPECIAL_TYPEDEF);
  ast->as.tdef.factor = glms_parser_parse_factor(parser);
  ast->as.tdef.id = glms_parser_parse_id(parser, true);

  const char *name = glms_ast_get_name(ast->as.tdef.id);

  if (name != 0) {
    hashy_map_set(&parser->symbols, name, ast);
  }

  return ast;
}

GLMSAST *glms_parser_parse_compound(GLMSParser *parser, bool skip_brace) {

  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_COMPOUND);

  while (parser->token.type != GLMS_TOKEN_TYPE_EOF && parser->error == false) {

    while (parser->token.type != GLMS_TOKEN_TYPE_SEMI) {
      GLMSAST *child = glms_parser_parse_expr(parser);

      if (!child)
        break;

      glms_ast_push(ast, child);

      if (child->type == GLMS_AST_TYPE_EOF)
        break;
    }

    while (parser->token.type == GLMS_TOKEN_TYPE_SEMI) {
      glms_parser_eat(parser, parser->token.type);
    }

    if (skip_brace && parser->token.type == GLMS_TOKEN_TYPE_RBRACE)
      break;
  }

  return ast;
}

GLMSAST *glms_parser_parse(GLMSParser *parser) {
  if (!parser)
    return 0;
  if (!parser->initialized)
    GLMS_WARNING_RETURN(0, stderr, "parser not initialized.\n");

  return glms_parser_parse_compound(parser, false);
}

bool glms_parser_peek_check_arrow_function(GLMSParser *parser) {
  GLMSLexer *lexer = &parser->env->lexer;
  int64_t start_i = lexer->i;
  char start_c = lexer->c;

  char c = lexer->source[lexer->i];
  int64_t i = lexer->i;
  while (c != 0 && c != ';') {

    if (c == '=' && lexer->source[MIN(i, lexer->length - 1)] == '>') {
      return true;
    }
    c = lexer->source[i++];
  }

  return false;
}

GLMSAST *glms_parser_lookup(GLMSParser *parser, const char *key) {
  if (!key || !parser)
    return 0;

  GLMSAST *g = (GLMSAST *)hashy_map_get(&parser->env->globals, key);

  if (g)
    return g;

  return (GLMSAST *)hashy_map_get(&parser->symbols, key);
}
