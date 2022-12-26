#ifndef GLMS_ENV_H
#define GLMS_ENV_H
#include <memo/memo.h>
#include <glms/ast.h>
#include <glms/lexer.h>
#include <glms/parser.h>
#include <glms/eval.h>
#include <glms/fptr.h>
#include <glms/stack.h>
#include <hashy/hashy.h>
#include <stdbool.h>

typedef struct {
  bool debug;
} GLMSConfig;

typedef struct GLMS_ENV_STRUCT {
  Memo memo_ast;
  GLMSConfig config;
  bool initialized;
  const char* source;

  GLMSLexer lexer;
  GLMSParser parser;
  GLMSEval eval;
  GLMSStack stack;

  HashyMap globals;


  GLMSAST* undefined;
} GLMSEnv;


int glms_env_init(
  GLMSEnv* env,
  const char* source,
  GLMSConfig cfg
);

GLMSAST* glms_env_new_ast(GLMSEnv* env, GLMSASTType type);

GLMSAST* glms_env_new_ast_number(GLMSEnv* env, float v);

GLMSAST* glms_env_new_ast_string(GLMSEnv* env, const char* value);

GLMSAST* glms_env_exec(GLMSEnv* env);

GLMSAST* glms_env_register_function(GLMSEnv* env, const char* name, GLMSFPTR fptr);

GLMSAST* glms_env_lookup_function(GLMSEnv* env, const char* name);

#endif
