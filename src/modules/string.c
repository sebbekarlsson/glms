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

void glms_string_constructor(GLMSEval *eval, GLMSStack *stack,
                                  GLMSASTBuffer *args, GLMSAST *self) {

  if (!self) return;
  self->type = GLMS_AST_TYPE_STRING;
  self->constructor = glms_string_constructor;
  self->to_string = 0;

  glms_ast_register_operator_overload(eval->env, self, GLMS_TOKEN_TYPE_ADD, glms_string_type_op_overload_add);
}

void glms_string_type(GLMSEnv *env) {
  glms_env_register_type(env, "string", glms_env_new_ast(env, GLMS_AST_TYPE_STRING, false), glms_string_constructor, 0, 0, 0);
  glms_env_register_type(env, GLMS_AST_TYPE_STR[GLMS_AST_TYPE_STRING], glms_env_new_ast(env, GLMS_AST_TYPE_STRING, false), glms_string_constructor, 0, 0, 0);
}
