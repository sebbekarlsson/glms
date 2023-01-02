#include "glms/allocator.h"
#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/env.h"
#include "glms/eval.h"
#include "glms/macros.h"
#include <glms/modules/iterator.h>

// typedef char* (*GLMSASTToString)(struct GLMS_AST_STRUCT *ast, GLMSAllocator alloc);

char *glms_iterator_to_string(GLMSAST *ast, GLMSAllocator alloc) {
  return 0;
}

int glms_iterator_fptr_next(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {

  if (!ast->iterator_next) {
    GLMS_WARNING_RETURN(0, stderr, "iterator_next == null.\n");
  }


  return glms_ast_iterate(eval->env, ast, &ast->as.iterator.it, out);
}

void glms_iterator_constructor(GLMSEval *eval, GLMSStack *stack,
                                  GLMSASTBuffer *args, GLMSAST *self) {

  if (!self) return;
  self->type = GLMS_AST_TYPE_ITERATOR;
  self->constructor = glms_iterator_constructor;
  // self->to_string = glms_iterator_to_string;
  glms_ast_register_function(eval->env, self, "next", glms_iterator_fptr_next);
}

void glms_iterator_type(GLMSEnv *env) {
  glms_env_register_type(env, "iterator", glms_env_new_ast(env, GLMS_AST_TYPE_ITERATOR, false), glms_iterator_constructor, 0, 0, 0);
}
