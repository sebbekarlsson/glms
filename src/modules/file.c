#include "glms/allocator.h"
#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/env.h"
#include "glms/eval.h"
#include "glms/macros.h"
#include "glms/string_view.h"
#include <glms/modules/file.h>
#include <stdio.h>

// typedef char* (*GLMSASTToString)(struct GLMS_AST_STRUCT *ast, GLMSAllocator alloc);

char *glms_file_to_string(GLMSAST *ast, GLMSAllocator alloc) {
  return 0;
}

int glms_file_fptr_open(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {

  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_STRING, GLMS_AST_TYPE_STRING }, 2, args)) return 0;



  const char* filepath = glms_string_view_get_value(&args->items[0].as.string.value);
  const char* mode = glms_string_view_get_value(&args->items[1].as.string.value);

  FILE* fp = fopen(filepath, mode);

  if (!fp) {
    GLMS_WARNING(stderr, "Failed to open `%s`.\n", filepath);
    return 0;
  }

  GLMSAST* file_ast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_STRUCT, true);
  file_ast->ptr = fp;
  file_ast->constructor = glms_file_constructor;
  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = file_ast };

  return 1;
}

int glms_file_fptr_close(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {


  if (!ast->ptr) GLMS_WARNING_RETURN(0, stderr, "file handle not open.\n");

  FILE* fp = (FILE*)ast->ptr;
  fclose(fp);
  return 1;
}

int glms_file_fptr_write(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {

  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_STRING}, 1, args)) return 0;
  if (!ast->ptr) GLMS_WARNING_RETURN(0, stderr, "file handle not open.\n");
  const char* buff = glms_string_view_get_value(&args->items[0].as.string.value);

  FILE* fp = (FILE*)ast->ptr;

  fwrite(buff, sizeof(char), strlen(buff), fp);
  
  
  return 1;
}

void glms_file_constructor(GLMSEval *eval, GLMSStack *stack,
                                  GLMSASTBuffer *args, GLMSAST *self) {

  if (!self) return;
  self->type = GLMS_AST_TYPE_STRUCT;
  self->constructor = glms_file_constructor;
  self->to_string = glms_file_to_string;
  glms_ast_register_function(eval->env, self, "open", glms_file_fptr_open);
  glms_ast_register_function(eval->env, self, "close", glms_file_fptr_close);
  glms_ast_register_function(eval->env, self, "write", glms_file_fptr_write);

  glms_env_register_function_signature(
    eval->env,
    self,
    "open",
    (GLMSFunctionSignature){
      .return_type = (GLMSType){ .typename = "file" },
      .args = (GLMSType[]){ (GLMSType){ GLMS_AST_TYPE_STRING, .valuename = "filename" }, (GLMSType){ GLMS_AST_TYPE_STRING, .valuename = "mode" }},
      .args_length = 2
    }
  );

  glms_env_register_function_signature(
    eval->env,
    self,
    "close",
    (GLMSFunctionSignature){
      .return_type = (GLMSType){ GLMS_AST_TYPE_BOOL },
      .args_length = 0
    }
  );

  glms_env_register_function_signature(
    eval->env,
    self,
    "write",
    (GLMSFunctionSignature){
      .return_type = (GLMSType){ GLMS_AST_TYPE_BOOL },
      .args = (GLMSType[]){ (GLMSType){ GLMS_AST_TYPE_STRING, .valuename = "text" }},
      .args_length = 1
    }
  );
}

void glms_file_type(GLMSEnv *env) {
  glms_env_register_type(env, "file", glms_env_new_ast(env, GLMS_AST_TYPE_STRUCT, false), glms_file_constructor, 0, 0/*glms_file_to_string*/, 0);
}
