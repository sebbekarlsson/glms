#include <jscript/env.h>
#include <jscript/constants.h>
#include <jscript/macros.h>
#include <jscript/io.h>
#include <jscript/builtin.h>


int jscript_env_init(
  JSCRIPTEnv* env,
  const char* source,
  JSCRIPTConfig cfg
) {
  if (!env) return 0;
  if (env->initialized) return 1;
  env->initialized = true;
  env->config = cfg;
  env->source = source;
  hashy_map_init(&env->globals, 256);
  memo_init(&env->memo_ast, (MemoConfig){ .item_size = sizeof(JSCRIPTAST), .page_capacity = JSCRIPT_MEMO_AST_PAGE_CAPACITY });


  env->undefined = jscript_env_new_ast(env, JSCRIPT_AST_TYPE_UNDEFINED);

  jscript_lexer_init(&env->lexer, env->source);
  jscript_parser_init(&env->parser, env);
  jscript_eval_init(&env->eval, env);
  jscript_builtin_init(env);


  return 1;
}

JSCRIPTAST* jscript_env_new_ast(JSCRIPTEnv* env, JSCRIPTASTType type) {
  if (!env) return 0;
  if (!env->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "env not initialized.\n");

  JSCRIPTAST* ast = (JSCRIPTAST*)memo_malloc(&env->memo_ast);
  if (!ast) JSCRIPT_WARNING_RETURN(0, stderr, "Failed to allocate AST.\n");

  ast->type = type;
  return ast;
}

JSCRIPTAST* jscript_env_new_ast_number(JSCRIPTEnv* env, float v) {
  JSCRIPTAST* ast = jscript_env_new_ast(env, JSCRIPT_AST_TYPE_NUMBER);
  ast->as.number.value = v;
  return ast;
}

JSCRIPTAST* jscript_env_new_ast_string(JSCRIPTEnv* env, const char* value) {
  JSCRIPTAST* ast = jscript_env_new_ast(env, JSCRIPT_AST_TYPE_STRING);

  if (value != 0) {
    ast->as.string.heap = strdup(value);
  }

  return ast;
}

JSCRIPTAST* jscript_env_exec(JSCRIPTEnv* env) {
  if (!env) return 0;
  if (!env->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "env not initialized.\n");

  JSCRIPTAST* root = jscript_parser_parse(&env->parser);
  jscript_stack_init(&env->stack);
  root = jscript_eval(&env->eval, root, &env->stack);
  return root;
}

JSCRIPTAST* jscript_env_register_function(JSCRIPTEnv* env, const char* name, JSCRIPTFPTR fptr) {
  if (!env || !name || !fptr) return 0;
  if (!env->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "env not initialized.\n");

  JSCRIPTAST* func = jscript_env_new_ast(env, JSCRIPT_AST_TYPE_FUNC);
  func->fptr = fptr;
  hashy_map_set(&env->globals, name, func);

  return func;
}

JSCRIPTAST* jscript_env_lookup_function(JSCRIPTEnv* env, const char* name) {
  if (!env || !name) return 0;
  if (!env->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "env not initialized.\n");

  return (JSCRIPTAST*)hashy_map_get(&env->globals, name);
}
