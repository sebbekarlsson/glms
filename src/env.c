#include <glms/builtin.h>
#include <glms/constants.h>
#include <glms/env.h>
#include <glms/io.h>
#include <glms/macros.h>

int glms_env_init(GLMSEnv *env, const char *source, GLMSConfig cfg) {
  if (!env)
    return 0;
  if (env->initialized)
    return 1;
  env->initialized = true;
  env->config = cfg;
  env->source = source;
  hashy_map_init(&env->globals, 256);
  memo_init(&env->memo_ast,
            (MemoConfig){.item_size = sizeof(GLMSAST),
                         .page_capacity = GLMS_MEMO_AST_PAGE_CAPACITY});

  env->undefined = glms_env_new_ast(env, GLMS_AST_TYPE_UNDEFINED);

  glms_lexer_init(&env->lexer, env->source);
  glms_parser_init(&env->parser, env);
  glms_eval_init(&env->eval, env);
  glms_builtin_init(env);

  return 1;
}

GLMSAST *glms_env_new_ast(GLMSEnv *env, GLMSASTType type) {
  if (!env)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  GLMSAST *ast = (GLMSAST *)memo_malloc(&env->memo_ast);
  if (!ast)
    GLMS_WARNING_RETURN(0, stderr, "Failed to allocate AST.\n");

  ast->type = type;
  return ast;
}

GLMSAST *glms_env_new_ast_number(GLMSEnv *env, float v) {
  GLMSAST *ast = glms_env_new_ast(env, GLMS_AST_TYPE_NUMBER);
  ast->as.number.value = v;
  return ast;
}

GLMSAST *glms_env_new_ast_string(GLMSEnv *env, const char *value) {
  GLMSAST *ast = glms_env_new_ast(env, GLMS_AST_TYPE_STRING);

  if (value != 0) {
    ast->as.string.heap = strdup(value);
  }

  return ast;
}

GLMSAST *glms_env_exec(GLMSEnv *env) {
  if (!env)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  GLMSAST *root = glms_parser_parse(&env->parser);
  glms_stack_init(&env->stack);
  root = glms_eval(&env->eval, root, &env->stack);
  return root;
}

GLMSAST *glms_env_register_function(GLMSEnv *env, const char *name,
                                    GLMSFPTR fptr) {
  if (!env || !name || !fptr)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  GLMSAST *func = glms_env_new_ast(env, GLMS_AST_TYPE_FUNC);
  func->fptr = fptr;
  hashy_map_set(&env->globals, name, func);

  return func;
}

GLMSAST *glms_env_lookup_function(GLMSEnv *env, const char *name) {
  if (!env || !name)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  return (GLMSAST *)hashy_map_get(&env->globals, name);
}
