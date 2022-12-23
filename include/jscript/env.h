#ifndef JSCRIPT_ENV_H
#define JSCRIPT_ENV_H
#include <memo/memo.h>
#include <jscript/ast.h>
#include <jscript/lexer.h>
#include <jscript/parser.h>
#include <jscript/eval.h>
#include <jscript/fptr.h>
#include <jscript/stack.h>
#include <hashy/hashy.h>
#include <stdbool.h>

typedef struct {
  bool debug;
} JSCRIPTConfig;

typedef struct JSCRIPT_ENV_STRUCT {
  Memo memo_ast;
  JSCRIPTConfig config;
  bool initialized;
  const char* source;

  JSCRIPTLexer lexer;
  JSCRIPTParser parser;
  JSCRIPTEval eval;
  JSCRIPTStack stack;

  HashyMap globals;
} JSCRIPTEnv;


int jscript_env_init(
  JSCRIPTEnv* env,
  const char* source,
  JSCRIPTConfig cfg
);

JSCRIPTAST* jscript_env_new_ast(JSCRIPTEnv* env, JSCRIPTASTType type);

JSCRIPTAST* jscript_env_new_ast_number(JSCRIPTEnv* env, float v);

JSCRIPTAST* jscript_env_new_ast_string(JSCRIPTEnv* env, const char* value);

JSCRIPTAST* jscript_env_exec(JSCRIPTEnv* env);

JSCRIPTAST* jscript_env_register_function(JSCRIPTEnv* env, const char* name, JSCRIPTFPTR fptr);

JSCRIPTAST* jscript_env_lookup_function(JSCRIPTEnv* env, const char* name);

#endif
