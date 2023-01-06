#include "glms/allocator.h"
#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/env.h"
#include "glms/eval.h"
#include <glms/modules/array.h>

// typedef char* (*GLMSASTToString)(struct GLMS_AST_STRUCT *ast, GLMSAllocator alloc);

char *glms_array_to_string(GLMSAST *ast, GLMSAllocator alloc, GLMSEnv* env) {
  char* s = 0;
  alloc.strcat(alloc.user_ptr, &s, "[");

  if (ast->children != 0 && ast->children->length > 0) {
    for (int64_t i = 0; i < ast->children->length; i++) {
	GLMSAST* child = ast->children->items[i];
	char* childstr = glms_ast_to_string(*child, alloc, env);
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

int glms_array_fptr_filter(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
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

    GLMSAST result = glms_eval(eval, glms_eval_call_func(eval, stack, &func, call_args), stack);

    if (glms_ast_is_truthy(result)) {
      glms_ast_push(new_array, val);
    }
  }


  ptr.as.stackptr.ptr = new_array;

  *out = ptr;

  return 1;
}

int glms_array_fptr_sort(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {

  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_FUNC }, 1, args)) return 0;


  GLMSAST* new_array = glms_ast_copy(*ast, eval->env);

  GLMSAST ptr = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_array };

  if (!ast->children || ast->children->length <= 0) {
    *out = ptr;
    return 1;
  }

  GLMSAST func = args->items[0];

  int64_t n = ast->children->length; 

  for (int64_t c = 0; c < n-1; c++) {

    for (int64_t d = 0; d < n - c - 1; d++) {
	GLMSASTBuffer call_args = (GLMSASTBuffer){
	    .initialized = true,
	    .items = (GLMSAST[]){ *new_array->children->items[d], *new_array->children->items[d+1] },
	    .length = 2
	};

	GLMSAST result = glms_eval(eval, glms_eval_call_func(eval, stack, &func, call_args), stack);

	if (glms_ast_is_truthy(result)) {
	  GLMSAST* swap = new_array->children->items[d];
	  new_array->children->items[d] = new_array->children->items[d+1];
	  new_array->children->items[d+1] = swap;
	}
    }
  }


  ptr.as.stackptr.ptr = new_array;

  *out = ptr;

  return 1;
}

int glms_array_fptr_push(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                         GLMSStack *stack, GLMSAST *out) {

  if (!args || args->length <= 0) return 0;

  for (int64_t i = 0; i < args->length; i++) {
    GLMSAST arg = args->items[i];
    GLMSAST* copy = glms_ast_copy(arg, eval->env);
    glms_ast_push(ast, copy);
  }

  return 1;
}

int glms_array_fptr_length(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                         GLMSStack *stack, GLMSAST *out) {

  int64_t len = ast->children ? ast->children->length : 0;
  *out = (GLMSAST){ .type = GLMS_AST_TYPE_NUMBER, .as.number.value = (float)len };
  return 1;
}

int glms_array_fptr_includes(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                         GLMSStack *stack, GLMSAST *out) {

  int64_t len = ast->children ? ast->children->length : 0;

  if (len <= 0 || (args == 0 || args->length <= 0 || args->items == 0)) {
    *out = (GLMSAST){ .type = GLMS_AST_TYPE_BOOL, .as.boolean = false };
    return 1;
  }


  GLMSAST compare = args->items[0];

  for (int64_t i = 0; i < len; i++) {
    GLMSAST* child = ast->children->items[i];

    if (glms_ast_compare_equals_equals(*child, compare)) {
      *out = (GLMSAST){ .type = GLMS_AST_TYPE_BOOL, .as.boolean = true };
      return 1;
    }
  }

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_BOOL, .as.boolean = false };
  
  return 1;
}

void glms_array_constructor(GLMSEval *eval, GLMSStack *stack,
                                  GLMSASTBuffer *args, GLMSAST *self) {

  if (!self) return;
  self->type = GLMS_AST_TYPE_ARRAY;
  self->constructor = glms_array_constructor;
  self->to_string = glms_array_to_string;
  glms_ast_register_function(eval->env, self, "map", glms_array_fptr_map);
  glms_ast_register_function(eval->env, self, "filter", glms_array_fptr_filter);
  glms_ast_register_function(eval->env, self, "sort", glms_array_fptr_sort);
  glms_ast_register_function(eval->env, self, "push", glms_array_fptr_push);
  glms_ast_register_function(eval->env, self, "length", glms_array_fptr_length);
  glms_ast_register_function(eval->env, self, "count", glms_array_fptr_length);
  glms_ast_register_function(eval->env, self, "includes", glms_array_fptr_includes);
}

void glms_array_type(GLMSEnv *env) {
  GLMSAST* t = glms_env_new_ast(env, GLMS_AST_TYPE_ARRAY, false);
  glms_env_register_type(env, "array", t, glms_array_constructor, 0, glms_array_to_string, 0);
  glms_env_register_type(env, GLMS_AST_TYPE_STR[GLMS_AST_TYPE_ARRAY], t, glms_array_constructor, 0, glms_array_to_string, 0);
}
