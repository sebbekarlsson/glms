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

int glms_env_clear(GLMSEnv* env);

GLMSAST* glms_env_new_ast(GLMSEnv* env, GLMSASTType type);

GLMSAST* glms_env_new_ast_make(GLMSEnv* env, GLMSAST ast);

GLMSAST* glms_env_new_ast_number(GLMSEnv* env, float v);

GLMSAST* glms_env_new_ast_string(GLMSEnv* env, const char* value);

GLMSAST* glms_env_new_ast_field(GLMSEnv* env, GLMSTokenType data_type, const char* name);

GLMSAST* glms_env_exec(GLMSEnv* env);

GLMSAST* glms_env_register_function(GLMSEnv* env, const char* name, GLMSFPTR fptr);

GLMSAST *glms_env_register_struct(GLMSEnv *env, const char *name,
                                  GLMSAST **fields, int fields_length);


GLMSAST *glms_env_register_any(GLMSEnv *env, const char *name,
                                  GLMSAST* ast);

GLMSAST *glms_env_register_type(
				GLMSEnv *env,
				const char *name,
				GLMSAST* ast,
				GLMSASTContructor constructor,
				GLMSASTSwizzle swizzle,
				GLMSASTToString to_string
				);

GLMSAST* glms_env_lookup_function(GLMSEnv* env, const char* name);

#endif
