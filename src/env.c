#include <jscript/env.h>
#include <jscript/constants.h>
#include <jscript/macros.h>


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

  memo_init(&env->memo_ast, (MemoConfig){ .item_size = sizeof(JSCRIPTAST), .page_capacity = JSCRIPT_MEMO_AST_PAGE_CAPACITY });
  jscript_lexer_init(&env->lexer, env->source);
  jscript_parser_init(&env->parser, env);

  return 1;
}

JSCRIPTAST* jscript_env_new_ast(JSCRIPTEnv* env, JSCRIPTASTType type) {
  if (!env) return 0;
  if (!env->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "env not initialized.\n");

  JSCRIPTAST* ast = (JSCRIPTAST*)memo_malloc(&env->memo_ast);

  ast->type = type;
  return ast;
}

JSCRIPTAST* jscript_env_exec(JSCRIPTEnv* env) {
  if (!env) return 0;
  if (!env->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "env not initialized.\n");

  jscript_parser_parse(&env->parser);

  return 0;
}
