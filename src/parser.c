#include <glms/env.h>
#include <glms/macros.h>
#include <glms/parser.h>
#include <stdio.h>
#include <string.h>
#include <text/text.h>

#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/lexer.h"
#include "glms/string_view.h"
#include "glms/token.h"

int glms_parser_init(GLMSParser *parser, GLMSEnv *env) {
  if (!parser || !env)
    return 0;
  if (parser->initialized)
    return 1;

  parser->initialized = true;
  parser->env = env;
  glms_lexer_next(&parser->env->lexer, &parser->token);

  if (!parser->symbols.initialized) {
    hashy_map_init(&parser->symbols, 256);
  }

  const char *tokname = glms_string_view_get_value(&parser->token.value);
  GLMSAST *known = glms_parser_lookup(parser, tokname);

  if (known) {
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

  if (!glms_lexer_next(&parser->env->lexer, &parser->token)) {
    parser->finished = true;
    return 0;
  }

  const char *tokname = glms_string_view_get_value(&parser->token.value);
  GLMSAST *known = glms_parser_lookup(parser, tokname);

  if (known) {
    parser->token.type = GLMS_TOKEN_TYPE_SPECIAL_USER_TYPE;
  }

  return 1;
}

static GLMSAST *glms_parser_error(GLMSParser *parser) {
  GLMS_WARNING(stderr, "Unexpected token `%s`\n",
               GLMS_TOKEN_TYPE_STR[parser->token.type]);
  parser->error = true;

  return glms_env_new_ast(parser->env, GLMS_AST_TYPE_EOF, false);
}

static GLMSAST *glms_parser_parse_noop(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_NOOP, false);
  return ast;
}

static GLMSAST *glms_parser_parse_eof(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_EOF, false);
  return ast;
}

GLMSAST *glms_parser_parse_id(GLMSParser *parser, bool skip_next) {
  GLMSASTList *flags = 0;

  if (glms_token_type_is_flag(parser->token.type)) {
    flags = NEW(GLMSASTList);
    glms_GLMSAST_list_init(flags);

    while (glms_token_type_is_flag(parser->token.type)) {
      GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_ID, false);
      ast->as.id.op = parser->token.type;
      ast->as.id.value = parser->token.value;
      glms_parser_eat(parser, parser->token.type);
      glms_GLMSAST_list_push(flags, ast);
    }
  }

  GLMSAST *id_ast = 0;

  if (parser->token.type == GLMS_TOKEN_TYPE_ID) {
    id_ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_ID, false);
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

GLMSAST *glms_parser_parse_null(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_NULL, false);
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_SPECIAL_NULL);
  return ast;
}

GLMSAST *glms_parser_parse_number(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_NUMBER, false);
  ast->as.number.value = atof(glms_string_view_get_value(&parser->token.value));
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_NUMBER);
  return ast;
}

GLMSAST *glms_parser_parse_int(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_NUMBER, false);
  ast->as.number.value = atoi(glms_string_view_get_value(&parser->token.value));
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_INT);
  return ast;
}
GLMSAST *glms_parser_parse_float(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_NUMBER, false);
  ast->as.number.value = atof(glms_string_view_get_value(&parser->token.value));
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_FLOAT);
  return ast;
}

GLMSAST *glms_parser_parse_bool(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BOOL, false);
  ast->as.boolean =
      parser->token.type == GLMS_TOKEN_TYPE_SPECIAL_FALSE ? false : true;
  glms_parser_eat(parser, parser->token.type);
  return ast;
}

GLMSAST *glms_parser_parse_string(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_STRING, false);

  if (parser->env->config.use_heap_strings) {
    const char *strval = glms_string_view_get_value(&parser->token.value);
    ast->as.string.heap = strval ? strdup(strval) : 0;
  } else {
    ast->as.string.value = parser->token.value;
  }
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_STRING);
  return ast;
}

#define GLMS_TEMPLATE_PARTS_CAP 256

static int glms_extract_template_parts(GLMSEnv *env, const char *strval,
                                       GLMSAST **out, int *out_length) {

  if (!strval || !out || !out_length)
    return 0;

  *out_length = 0;
  int64_t len = strlen(strval);

  int i = 0;
  char c = strval[i++];

  int count = 0;

  char *next_part = 0;
  bool needs_parsing = false;

  while (c != 0) {

    while (c != '$' && c != 0) {
      text_append(&next_part, (char[]){c, 0});
      c = strval[i++];
    }

    if (c == 0)
      break;

    if (next_part != 0) {
      out[count++] = glms_env_new_ast_string(env, next_part, false);
      free(next_part);
      next_part = 0;
      c = strval[i++];
      c = strval[i++];
    }

    if (c == 0)
      break;

    while (c != '}' && c != 0) {
      text_append(&next_part, (char[]){c, 0});
      c = strval[i++];
    }

    if (c == '}') {
      c = strval[i++];
    }

    if (next_part != 0) {
      GLMSEnv tmp_env = {0};
      GLMSConfig cfg = env->config;
      cfg.memo_ast = &env->memo_ast;
      cfg.use_heap_strings = true;
      glms_env_init(&tmp_env, next_part, env->entry_path, cfg);
      GLMSAST *parsed = glms_parser_parse_expr(&tmp_env.parser);
      glms_env_clear(&tmp_env);

      if (parsed != 0) {
        parsed->env_ref = env;
        out[count++] = parsed;
      }

      next_part = 0;
    }
    c = strval[i++];
  }

  *out_length = count;

  return *out_length > 0;
}

GLMSAST *glms_parser_parse_template_string(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_STRING, false);

  if (parser->env->config.use_heap_strings) {
    const char *strval = glms_string_view_get_value(&parser->token.value);
    ast->as.string.heap = strval ? strdup(strval) : 0;
  } else {
    ast->as.string.value = parser->token.value;
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_TEMPLATE_STRING);

  const char *strval = glms_ast_get_string_value(ast);
  if (!strval || strchr(strval, '$') == 0)
    return ast;

  GLMSAST *parts[GLMS_TEMPLATE_PARTS_CAP];
  int len = 0;
  if (glms_extract_template_parts(parser->env, strval, parts, &len)) {

    for (int i = 0; i < len; i++) {
      glms_ast_push(ast, parts[i]);
    }
  }

  return ast;
}

GLMSAST *glms_parser_parse_array(GLMSParser *parser) {
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LBRACKET);

  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_ARRAY, false);

  if (parser->token.type != GLMS_TOKEN_TYPE_RBRACKET) {
    GLMSAST *arg = glms_parser_parse_expr(parser);
    glms_ast_push(ast, arg);

    while (parser->token.type == GLMS_TOKEN_TYPE_COMMA) {
      glms_parser_eat(parser, GLMS_TOKEN_TYPE_COMMA);
      GLMSAST *arg = glms_parser_parse_expr(parser);
      glms_ast_push(ast, arg);
    }
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
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_OBJECT, false);

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
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BINOP, false);
  ast->as.binop.left = left;
  ast->as.binop.op = parser->token.type;
  glms_parser_eat(parser, parser->token.type);
  ast->as.binop.right = glms_parser_parse_term(parser);
  return ast;
}

GLMSAST *glms_parser_parse_struct(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_STRUCT, false);
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

GLMSAST *glms_parser_parse_enum(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_STRUCT, false);
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_SPECIAL_ENUM);

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LBRACE);

  int64_t v_idx = 0;

  if (parser->token.type != GLMS_TOKEN_TYPE_RBRACE) {
    GLMSAST *field = glms_parser_parse_expr(parser);
    const char *key = glms_ast_get_string_value(field);
    if (!key) {
      GLMS_WARNING(stderr, "key == null.\n");
    }

    GLMSAST *idx = glms_env_new_ast_number(parser->env, v_idx, false);
    glms_ast_object_set_property(ast, key, idx);
    v_idx++;
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_COMMA) {
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_COMMA);

    if (parser->token.type == GLMS_TOKEN_TYPE_RBRACE)
      break;

    GLMSAST *field = glms_parser_parse_expr(parser);
    const char *key = glms_ast_get_string_value(field);
    if (!key) {
      GLMS_WARNING(stderr, "key == null.\n");
      continue;
    }

    GLMSAST *idx = glms_env_new_ast_number(parser->env, v_idx, false);
    glms_ast_object_set_property(ast, key, idx);
    v_idx++;
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_RBRACE);

  return ast;
}

GLMSAST *glms_parser_parse_unop(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_UNOP, false);
  ast->as.unop.op = parser->token.type;
  glms_parser_eat(parser, parser->token.type);

  if (parser->token.type != GLMS_TOKEN_TYPE_SEMI) {
    ast->as.unop.right = glms_parser_parse_expr(parser);
  } else if (parser->token.type == GLMS_TOKEN_TYPE_SEMI) {
    glms_parser_eat(parser, parser->token.type);
  }
  return ast;
}

GLMSAST *glms_parser_parse_import(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_IMPORT, false);
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_SPECIAL_IMPORT);

  ast->as.import.value = parser->token.value;
  glms_parser_eat(parser, GLMS_TOKEN_TYPE_STRING);

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_SPECIAL_AS);

  ast->as.import.id = glms_parser_parse_id(parser, true);

  return ast;
}

GLMSAST *glms_parser_parse_factor(GLMSParser *parser) {
  if (parser->token.type == GLMS_TOKEN_TYPE_SUB ||
      parser->token.type == GLMS_TOKEN_TYPE_ADD ||
      parser->token.type == GLMS_TOKEN_TYPE_EXCLAM) {
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
  case GLMS_TOKEN_TYPE_SPECIAL_INT:
  case GLMS_TOKEN_TYPE_SPECIAL_FLOAT:
  case GLMS_TOKEN_TYPE_SPECIAL_ARRAY:
  case GLMS_TOKEN_TYPE_SPECIAL_OBJECT:
  case GLMS_TOKEN_TYPE_SPECIAL_BOOL:
  case GLMS_TOKEN_TYPE_SPECIAL_USER_TYPE:
  case GLMS_TOKEN_TYPE_ID: {
    return glms_parser_parse_id(parser, false);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_NULL: {
    return glms_parser_parse_null(parser);
  }; break;
  case GLMS_TOKEN_TYPE_STRING: {
    return glms_parser_parse_string(parser);
  }; break;
  case GLMS_TOKEN_TYPE_TEMPLATE_STRING: {
    return glms_parser_parse_template_string(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_IMPORT: {
    return glms_parser_parse_import(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_FALSE:
  case GLMS_TOKEN_TYPE_SPECIAL_TRUE: {
    return glms_parser_parse_bool(parser);
  }; break;
  case GLMS_TOKEN_TYPE_NUMBER: {
    return glms_parser_parse_number(parser);
  }; break;
  case GLMS_TOKEN_TYPE_INT: {
    return glms_parser_parse_int(parser);
  }; break;
  case GLMS_TOKEN_TYPE_FLOAT: {
    return glms_parser_parse_float(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_TYPEDEF: {
    return glms_parser_parse_typedef(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_STRUCT: {
    return glms_parser_parse_struct(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_ENUM: {
    return glms_parser_parse_enum(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_FUNCTION: {
    return glms_parser_parse_function(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_BREAK:
  case GLMS_TOKEN_TYPE_SPECIAL_RETURN: {
    return glms_parser_parse_unop(parser);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_SWITCH:
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

  while (left && parser->token.type == GLMS_TOKEN_TYPE_DOT) {
    GLMSAST *access =
        glms_env_new_ast(parser->env, GLMS_AST_TYPE_ACCESS, false);
    access->as.access.left = left;

    if (parser->token.type == GLMS_TOKEN_TYPE_DOT) {
      glms_parser_eat(parser, parser->token.type);
    }

    access->as.access.right = glms_parser_parse_factor(parser);
    left = access;
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_ADD_ADD ||
         parser->token.type == GLMS_TOKEN_TYPE_SUB_SUB) {
    GLMSAST *unop = glms_env_new_ast(parser->env, GLMS_AST_TYPE_UNOP, false);
    unop->as.unop.left = left;
    unop->as.unop.op = parser->token.type;
    glms_parser_eat(parser, parser->token.type);
    left = unop;
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_MUL ||
         parser->token.type == GLMS_TOKEN_TYPE_DIV) {
    GLMSAST *binop = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BINOP, false);
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
    GLMSAST *binop = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BINOP, false);
    binop->as.binop.left = left;
    binop->as.binop.op = parser->token.type;
    glms_parser_eat(parser, parser->token.type);
    binop->as.binop.right = glms_parser_parse_term(parser);
    left = binop;
  }

  return left;
}

GLMSAST *glms_parser_parse_expr(GLMSParser *parser) {
  GLMSAST *left = glms_parser_parse_term(parser);

  while (parser->token.type == GLMS_TOKEN_TYPE_LBRACKET) {
    GLMSAST *access =
        glms_env_new_ast(parser->env, GLMS_AST_TYPE_ACCESS, false);
    access->as.access.left = left;

    access->as.access.right = glms_parser_parse_expr(parser);
    left = access;
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_ADD ||
         parser->token.type == GLMS_TOKEN_TYPE_SUB ||
         parser->token.type == GLMS_TOKEN_TYPE_DIV_EQUALS ||
         parser->token.type == GLMS_TOKEN_TYPE_ADD_EQUALS ||
         parser->token.type == GLMS_TOKEN_TYPE_MUL_EQUALS ||
         parser->token.type == GLMS_TOKEN_TYPE_SUB_EQUALS) {
    GLMSAST *binop = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BINOP, false);
    binop->as.binop.left = left;
    binop->as.binop.op = parser->token.type;
    glms_parser_eat(parser, parser->token.type);
    binop->as.binop.right = glms_parser_parse_term(parser);
    left = binop;
  }

  while (left && parser->token.type == GLMS_TOKEN_TYPE_DOT) {
    GLMSAST *access =
        glms_env_new_ast(parser->env, GLMS_AST_TYPE_ACCESS, false);
    access->as.access.left = left;
    glms_parser_eat(parser, parser->token.type);
    access->as.access.right = glms_parser_parse_term(parser);
    left = access;
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_EQUALS) {
    GLMSAST *binop = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BINOP, false);
    binop->as.binop.left = left;
    binop->as.binop.op = parser->token.type;
    glms_parser_eat(parser, parser->token.type);
    GLMSAST *right = binop->as.binop.right = glms_parser_parse_expr(parser);

    if (right->type == GLMS_AST_TYPE_FUNC && left->type == GLMS_AST_TYPE_ID) {
      right->as.func.id = glms_ast_copy(*left, parser->env);
    }

    left = binop;
  }

  while (parser->token.type == GLMS_TOKEN_TYPE_AND_AND ||
         parser->token.type == GLMS_TOKEN_TYPE_PIPE_PIPE) {
    GLMSAST *binop = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BINOP, false);
    binop->as.binop.left = left;
    binop->as.binop.op = parser->token.type;
    glms_parser_eat(parser, parser->token.type);
    binop->as.binop.right = glms_parser_parse_expr(parser);
    left = binop;
  }

  return left;
}

GLMSAST *glms_parser_parse_call(GLMSParser *parser, GLMSAST *left) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_CALL, false);
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
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_FUNC, false);

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
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_FUNC, false);
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

GLMSAST *glms_parser_parse_switch_body(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_COMPOUND, false);

  while (parser->token.type == GLMS_TOKEN_TYPE_SPECIAL_CASE) {
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_SPECIAL_CASE);

    GLMSAST *cond = glms_parser_parse_expr(parser);

    glms_parser_eat(parser, GLMS_TOKEN_TYPE_COLON);

    GLMSAST *expr = glms_parser_parse_expr(parser);

    GLMSAST *block = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BLOCK, false);
    block->as.block.op = GLMS_TOKEN_TYPE_SPECIAL_CASE;
    block->as.block.expr = cond;
    block->as.block.body = expr;

    glms_ast_push(ast, block);
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_SEMI);

    if (parser->token.type == GLMS_TOKEN_TYPE_SPECIAL_BREAK) {
      glms_parser_eat(parser, GLMS_TOKEN_TYPE_SPECIAL_BREAK);
      glms_parser_eat(parser, GLMS_TOKEN_TYPE_SEMI);
    }
  }

  return ast;
}

GLMSAST *glms_parser_parse_block(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_BLOCK, false);
  ast->as.block.op = parser->token.type;
  glms_parser_eat(parser, parser->token.type);

  if (parser->token.type == GLMS_TOKEN_TYPE_SPECIAL_IF) {
    glms_parser_eat(parser, parser->token.type);
  }

  if (parser->token.type == GLMS_TOKEN_TYPE_LPAREN) {
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_LPAREN);
    ast->as.block.expr = glms_parser_parse_expr(parser);
    glms_parser_eat(parser, GLMS_TOKEN_TYPE_RPAREN);
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_LBRACE);

  if (parser->token.type != GLMS_TOKEN_TYPE_RBRACE) {
    ast->as.block.body = ast->as.block.op == GLMS_TOKEN_TYPE_SPECIAL_SWITCH
                             ? glms_parser_parse_switch_body(parser)
                             : glms_parser_parse_compound(parser, true);
  }

  glms_parser_eat(parser, GLMS_TOKEN_TYPE_RBRACE);

  if (parser->token.type == GLMS_TOKEN_TYPE_SPECIAL_ELSE) {
    ast->as.block.next = glms_parser_parse_block(parser);
  }

  return ast;
}

GLMSAST *glms_parser_parse_for(GLMSParser *parser) {
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_FOR, false);
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
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_TYPEDEF, false);
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
  GLMSAST *ast = glms_env_new_ast(parser->env, GLMS_AST_TYPE_COMPOUND, false);

  while (parser->token.type != GLMS_TOKEN_TYPE_EOF && parser->error == false &&
         parser->finished == false) {
    while (parser->token.type != GLMS_TOKEN_TYPE_SEMI &&
           parser->token.type != GLMS_TOKEN_TYPE_EOF &&
           parser->error == false && parser->finished == false) {
      GLMSAST *child = glms_parser_parse_expr(parser);

      if (!child)
        break;

      glms_ast_push(ast, child);

      if (child->type == GLMS_AST_TYPE_EOF)
        break;

      if (skip_brace && parser->token.type == GLMS_TOKEN_TYPE_RBRACE)
        break;
    }

    while (parser->token.type == GLMS_TOKEN_TYPE_SEMI &&
           parser->finished == false) {
      if (!glms_parser_eat(parser, parser->token.type))
        break;
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
