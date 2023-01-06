#include "fastjson/options.h"
#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/env.h"
#include "glms/eval.h"
#include "glms/type.h"
#include <glms/modules/json.h>
#include <curl/curl.h>
#include <glms/macros.h>
#include <fastjson/json.h>

int glms_json_fptr_parse(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                            GLMSStack *stack, GLMSAST *out) {


  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_STRING }, 1, args)) return 0;
  JSONOptions options = {0};
  options.optimized_strings = true;

  const char* contents = glms_ast_get_string_value(&args->items[0]);

  JSON* j = 0;
  if (!(j = json_parse(contents, &options))) GLMS_WARNING_RETURN(0, stderr, "Failed to parse json.\n");


  GLMSAST* new_ast = glms_ast_from_json(eval->env, j); 

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_ast };


  return 1;
}

int glms_json_fptr_stringify(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                            GLMSStack *stack, GLMSAST *out) {

  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_OBJECT }, 1, args)) return 0;

  GLMSAST obj = args->items[0];
  if (!obj.json) return 0;

  GLMSAST* new_ast = glms_env_new_ast_string(eval->env, json_stringify(obj.json), true);

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_ast };
  
  return 1;
}

void glms_json_constructor(GLMSEval *eval, GLMSStack *stack,
                               GLMSASTBuffer *args, GLMSAST *self) {

  if (self->constructed) return;
  self->constructed = true;

  self->constructor = glms_json_constructor;
  glms_ast_register_function(eval->env, self, "parse", glms_json_fptr_parse);
  glms_ast_register_function(eval->env, self, "stringify", glms_json_fptr_stringify);

  glms_env_register_function_signature(eval->env, self, "parse", (GLMSFunctionSignature){
      .return_type = (GLMSType){ GLMS_AST_TYPE_OBJECT },
      .args = (GLMSType[]) {
	(GLMSType){ GLMS_AST_TYPE_STRING, .valuename = "jsonString" }
      },
      .args_length = 1
  });

  glms_env_register_function_signature(eval->env, self, "stringify", (GLMSFunctionSignature){
      .return_type = (GLMSType){ GLMS_AST_TYPE_STRING },
      .args = (GLMSType[]) {
	(GLMSType){ GLMS_AST_TYPE_OBJECT }
      },
      .args_length = 1
  });
}

void glms_json(GLMSEnv *env) {

  GLMSAST* t = glms_env_new_ast(env, GLMS_AST_TYPE_STRUCT, false);
  t->constructor = glms_json_constructor;
  glms_env_register_type(env, "json", t, glms_json_constructor, 0, 0, 0);
}
