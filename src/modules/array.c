#include "glms/allocator.h"
#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/env.h"
#include "glms/eval.h"
#include <glms/modules/array.h>

// typedef char* (*GLMSASTToString)(struct GLMS_AST_STRUCT *ast, GLMSAllocator alloc);

char *glms_array_to_string(GLMSAST *ast, GLMSAllocator alloc) {
  char* s = 0;
  alloc.strcat(alloc.user_ptr, &s, "[");

  if (ast->children != 0 && ast->children->length > 0) {
    for (int64_t i = 0; i < ast->children->length; i++) {
	GLMSAST* child = ast->children->items[i];
	char* childstr = glms_ast_to_string(*child, alloc);
	if (childstr == 0) continue;
	alloc.strcat(alloc.user_ptr, &s, childstr);

	if (i < ast->children->length-1) {
	  alloc.strcat(alloc.user_ptr, &s, ", ");
	}
    }
  }
  
  alloc.strcat(alloc.user_ptr, &s, "]");

  return s;
}

int glms_array_fptr_map(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {

  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_FUNC }, 1, args)) return 0;


  GLMSAST* new_array = glms_env_new_ast(eval->env, GLMS_AST_TYPE_ARRAY, true);

  GLMSAST ptr = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_array };

  if (!ast->children || ast->children->length <= 0) {
    *out = ptr;
    return 1;
  }


  GLMSAST func = args->items[0];

  for (int64_t i = 0; i < ast->children->length; i++) {

    GLMSAST* val = ast->children->items[i];
    
    GLMSASTBuffer call_args = (GLMSASTBuffer){
      .initialized = true,
      .items = (GLMSAST[]){ *val },
      .length = 1
    };

    GLMSAST mapped = glms_eval(eval, glms_eval_call_func(eval, stack, &func, call_args), stack);

    glms_ast_push(new_array, glms_ast_copy(mapped, eval->env));
  }


  ptr.as.stackptr.ptr = new_array;

  *out = ptr;

  return 1;
}

void glms_array_constructor(GLMSEval *eval, GLMSStack *stack,
                                  GLMSASTBuffer *args, GLMSAST *self) {

  if (!self) return;
  self->type = GLMS_AST_TYPE_ARRAY;
  self->constructor = glms_array_constructor;
  self->to_string = glms_array_to_string;
  glms_ast_register_function(eval->env, self, "map", glms_array_fptr_map);
}

void glms_array_type(GLMSEnv *env) {
  glms_env_register_type(env, "array", glms_env_new_ast(env, GLMS_AST_TYPE_ARRAY, false), glms_array_constructor, 0, glms_array_to_string, 0);
}
