#include "glms/allocator.h"
#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/env.h"
#include "glms/eval.h"
#include "glms/string_view.h"
#include "glms/token.h"
#include <glms/modules/string.h>
#include <text/text.h>

// typedef char* (*GLMSASTToString)(struct GLMS_AST_STRUCT *ast, GLMSAllocator alloc);

char *glms_string_to_string(GLMSAST *ast, GLMSAllocator alloc) { return 0; }

int glms_string_type_op_overload_add(GLMSEval *eval, GLMSStack *stack,
                                     GLMSAST *left, GLMSAST *right,
                                     GLMSAST *out) {


  
  if (left->type != GLMS_AST_TYPE_STRING && right->type != GLMS_AST_TYPE_STRING)
    return 0;


  const char* left_value = glms_ast_get_string_value(left);
  const char* right_value = glms_ast_get_string_value(right);

  char* s = 0;
  if (left_value) {
    text_append(&s, left_value);
  }

  if (right_value) {
    text_append(&s, right_value);
  }

  GLMSAST* new_ast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_STRING, true);
  new_ast->as.string.heap = s;

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_ast };
  return 1;
}

int glms_string_fptr_replace(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {

  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_STRING, GLMS_AST_TYPE_STRING }, 2, args)) return 0;

  const char* needle = glms_ast_get_string_value(&args->items[0]);
  const char* repl = glms_ast_get_string_value(&args->items[1]);
  const char* value = glms_ast_get_string_value(ast);
  char* new_str = text_replace(value, needle, repl);
  if (!new_str) return 0;


  GLMSAST* new_ast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_STRING, true);
  new_ast->as.string.heap = new_str;

  
  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_ast };

  return 1;
}

int glms_string_fptr_includes(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {

  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_STRING }, 1, args)) return 0;

  const char* needle = glms_ast_get_string_value(&args->items[0]);
  const char* value = glms_ast_get_string_value(ast);


  if (!needle || !value) {

    *out = (GLMSAST){ .type = GLMS_AST_TYPE_BOOL, .as.boolean = false };
    return 1;
  }

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_BOOL, .as.boolean = (strstr(value, needle) != 0) };

  return 1;
}

void glms_string_constructor(GLMSEval *eval, GLMSStack *stack,
                                  GLMSASTBuffer *args, GLMSAST *self) {

  if (!self) return;
  self->type = GLMS_AST_TYPE_STRING;
  self->constructor = glms_string_constructor;
  self->to_string = 0;

  glms_ast_register_operator_overload(eval->env, self, GLMS_TOKEN_TYPE_ADD, glms_string_type_op_overload_add);
  glms_ast_register_function(eval->env, self, "replace", glms_string_fptr_replace);
  glms_ast_register_function(eval->env, self, "includes", glms_string_fptr_includes);

  glms_env_register_function_signature(
    eval->env,
    self,
    "replace",
    (GLMSFunctionSignature){
      .return_type = (GLMSType){ GLMS_AST_TYPE_STRING },
      .args = (GLMSType[]){ (GLMSType){ GLMS_AST_TYPE_STRING, .valuename = "pattern" }, (GLMSType){ GLMS_AST_TYPE_STRING, .valuename = "replacement" }},
      .args_length = 2
    }
  );

  glms_env_register_function_signature(
    eval->env,
    self,
    "includes",
    (GLMSFunctionSignature){
      .return_type = (GLMSType){ GLMS_AST_TYPE_BOOL },
      .args = (GLMSType[]){ (GLMSType){ GLMS_AST_TYPE_STRING, .valuename = "needle" }},
      .args_length = 1
    }
  );
}

void glms_string_type(GLMSEnv *env) {
  glms_env_register_type(env, "string", glms_env_new_ast(env, GLMS_AST_TYPE_STRING, false), glms_string_constructor, 0, 0, 0);
  glms_env_register_type(env, GLMS_AST_TYPE_STR[GLMS_AST_TYPE_STRING], glms_env_new_ast(env, GLMS_AST_TYPE_STRING, false), glms_string_constructor, 0, 0, 0);
}
