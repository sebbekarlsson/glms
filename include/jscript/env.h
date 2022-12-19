#ifndef JSCRIPT_ENV_H
#define JSCRIPT_ENV_H
#include <memo/memo.h>
#include <jscript/ast.h>
#include <jscript/lexer.h>
#include <stdbool.h>

typedef struct {

} JSCRIPTConfig;

typedef struct {
  Memo memo_ast;
  JSCRIPTConfig config;
  bool initialized;
  const char* source;

  JSCRIPTLexer lexer;
} JSCRIPTEnv;


int jscript_env_init(
  JSCRIPTEnv* env,
  const char* source,
  JSCRIPTConfig cfg
);

JSCRIPTAST* jscript_env_new_ast(JSCRIPTEnv* env);

JSCRIPTAST* jscript_env_exec(JSCRIPTEnv* env);

#endif
