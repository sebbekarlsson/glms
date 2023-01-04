#ifndef GLMS_ENV_H
#define GLMS_ENV_H
#include "glms/type.h"
#include <memo/memo.h>
#include <arena/arena.h>
#include <glms/ast.h>
#include <glms/lexer.h>
#include <glms/parser.h>
#include <glms/eval.h>
#include <glms/fptr.h>
#include <glms/stack.h>
#include <hashy/hashy.h>
#include <stdbool.h>
#include <glms/allocator.h>

typedef struct {
  bool debug;
} GLMSConfig;

typedef struct GLMS_ENV_STRUCT {
  Memo memo_ast;
  Arena arena_ast;
  GLMSConfig config;
  bool initialized;
  const char* source;
  const char* entry_path;

  GLMSLexer lexer;
  GLMSParser parser;
  GLMSEval eval;
  GLMSStack stack;

  HashyMap globals;
  HashyMap types;


  GLMSAST* undefined;
  GLMSAST* stackptr;

  bool use_arena;

  GLMSAllocator string_alloc;

  char* last_joined_path;

  GLMSAST* root;
} GLMSEnv;

typedef void (*GLMSExtensionEntryFunc)(GLMSEnv* env);

int glms_env_init(
  GLMSEnv* env,
  const char* source,
  const char* entry_path,
  GLMSConfig cfg
);

int glms_env_clear(GLMSEnv* env);

GLMSAST* glms_env_new_ast(GLMSEnv* env, GLMSASTType type, bool arena);

GLMSAST* glms_env_new_ast_make(GLMSEnv* env, GLMSAST ast, bool arena);

GLMSAST* glms_env_new_ast_number(GLMSEnv* env, float v, bool arena);

GLMSAST* glms_env_new_ast_vec3(GLMSEnv* env, Vector3 v, bool arena);

GLMSAST* glms_env_new_ast_string(GLMSEnv* env, const char* value, bool arena);

GLMSAST* glms_env_new_ast_field(GLMSEnv* env, GLMSTokenType data_type, const char* name, bool arena);

GLMSAST *glms_env_exec(GLMSEnv *env);

int glms_env_call_function(GLMSEnv* env, const char* name, GLMSASTBuffer args, GLMSAST* out);

GLMSAST *glms_env_register_function(GLMSEnv *env, const char *name,
                                    GLMSFPTR fptr);

int glms_env_register_function_signature(GLMSEnv *env, GLMSAST* ast, const char *name,
					 GLMSFunctionSignature signature);

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
				GLMSASTToString to_string,
				GLMSASTDestructor destructor
				);

GLMSAST *glms_env_lookup_function(GLMSEnv *env, const char *name);

GLMSAST *glms_env_lookup(GLMSEnv *env, const char *name);



GLMSAST *glms_env_lookup_type(GLMSEnv *env, const char *name);

GLMSAST *glms_env_get_type_for(GLMSEnv *env, GLMSAST* ast);


GLMSAST *glms_env_apply_type(GLMSEnv *env, GLMSEval *eval, GLMSStack *stack,
                             GLMSAST *ast);

const char *glms_env_get_path_for(GLMSEnv *env, const char *path);

int glms_env_export_docstrings(GLMSEnv* env, const char* filepath);

#endif
