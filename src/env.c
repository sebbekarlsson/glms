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

  return 1;
}

JSCRIPTAST* jscript_env_new_ast(JSCRIPTEnv* env) {
  if (!env) return 0;
  if (!env->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "env not initialized.\n");

  return (JSCRIPTAST*)memo_malloc(&env->memo_ast);
}

JSCRIPTAST* jscript_env_exec(JSCRIPTEnv* env) {
  if (!env) return 0;
  if (!env->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "env not initialized.\n");

  JSCRIPTToken token = {0};
  while (jscript_lexer_next(&env->lexer, &token)) {

    if (token.type == JSCRIPT_TOKEN_TYPE_ID) {
      const char* value = jscript_string_view_get_value(&token.value);
      printf("ID: (%s)\n", value);
    } else if (token.type == JSCRIPT_TOKEN_TYPE_NUMBER) {
      const char* value = jscript_string_view_get_value(&token.value);
      printf("Number: (%s)\n", value);
    } else {
      printf("Token: %c\n", token.c);
    }
  }

  return 0;
}
