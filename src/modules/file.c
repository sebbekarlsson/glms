#include "glms/allocator.h"
#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/env.h"
#include "glms/eval.h"
#include "glms/iterator.h"
#include "glms/macros.h"
#include "glms/string_view.h"
#include <glms/modules/file.h>
#include <glms/io.h>
#include <stdio.h>

// typedef char* (*GLMSASTToString)(struct GLMS_AST_STRUCT *ast, GLMSAllocator alloc);


int glms_file_fptr_open(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {

  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_STRING, GLMS_AST_TYPE_STRING }, 2, args)) return 0;



  const char* filepath = glms_string_view_get_value(&args->items[0].as.string.value);

  if (!glms_file_exists(filepath)) {
    const char* nextpath = glms_env_get_path_for(eval->env, filepath);
    if (nextpath != 0) filepath = nextpath;
  }
  
  
  const char* mode = glms_string_view_get_value(&args->items[1].as.string.value);

  FILE* fp = fopen(filepath, mode);

  if (!fp) {
    GLMS_WARNING(stderr, "Failed to open `%s`.\n", filepath);
    return 0;
  }

  GLMSAST* file_ast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_STRUCT, true);

  GLMSFile* f = NEW(GLMSFile);
  f->fp = fp;
  f->open = true;
  file_ast->ptr = f;
  file_ast->constructor = glms_file_constructor;
  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = file_ast };

  return 1;
}

int glms_file_fptr_close(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {


  if (!ast->ptr) GLMS_WARNING_RETURN(0, stderr, "file handle not open.\n");

  GLMSFile* f = (GLMSFile*)ast->ptr;

  if (!f->fp) GLMS_WARNING_RETURN(0, stderr, "file handle not open.\n");
  
  FILE* fp = f->fp;
  fclose(fp);
  f->fp = 0;
  f->open = false;
  return 1;
}

int glms_file_fptr_write(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {

  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_STRING}, 1, args)) return 0;
  if (!ast->ptr) GLMS_WARNING_RETURN(0, stderr, "file handle not open.\n");
  const char* buff = glms_string_view_get_value(&args->items[0].as.string.value);

  GLMSFile* f = (GLMSFile*)ast->ptr;
  if (!f->fp) GLMS_WARNING_RETURN(0, stderr, "file handle not open.\n");
  
  FILE* fp = f->fp;

  fwrite(buff, sizeof(char), strlen(buff), fp);
  
  
  return 1;
}


int glms_file_iterator_next(GLMSEnv* env, GLMSAST *self, GLMSIterator *it, GLMSAST *out) {
  GLMSFileIteratorState* state = (GLMSFileIteratorState*)self->as.iterator.state;
  GLMSFile* f =  state->file;


  char* line = state->line;


    size_t len = 0;
    ssize_t read = 0;



     if((read = getline(&line, &len, f->fp)) != -1) {
      state->read_bytes += read;
      GLMSAST* strast = glms_env_new_ast(env, GLMS_AST_TYPE_STRING, true);
      strast->as.string.heap = strdup(line);
      *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = strast };
      free(state->line);
      state->line = 0;
    } else {
      *out = (GLMSAST){ .type = GLMS_AST_TYPE_NULL };

      if (state->line != 0) {
	free(state->line);
	state->line =0;
      }
      return 1;
    }

    return 1;
}

int glms_file_fptr_read_lines(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {

  if (!ast->ptr) GLMS_WARNING_RETURN(0, stderr, "file handle not open.\n");

  GLMSFile* f = (GLMSFile*)ast->ptr;
  if (!f->fp) GLMS_WARNING_RETURN(0, stderr, "file handle not open.\n");


  GLMSAST* iter_ast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_ITERATOR, true);
  iter_ast->as.iterator.it = (GLMSIterator){0};
  iter_ast->iterator_next = glms_file_iterator_next;

  GLMSFileIteratorState* state = NEW(GLMSFileIteratorState);
  state->file = f;
  state->last_ptr = 0;
  state->line = 0;
  state->pos = 0;
  iter_ast->as.iterator.state = state;

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = iter_ast };
  return 1;
}

int glms_file_fptr_read(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {

  if (!ast->ptr) GLMS_WARNING_RETURN(0, stderr, "file handle not open.\n");

  GLMSFile* f = (GLMSFile*)ast->ptr;
  if (!f->fp) GLMS_WARNING_RETURN(0, stderr, "file handle not open.\n");


  FILE* fp = f->fp;


   char *buffer = NULL;
  size_t len;
  ssize_t bytes_read = getdelim(&buffer, &len, '\0', fp);
  if (bytes_read == -1) {
    return 0;
  }


  GLMSAST* new_ast = glms_env_new_ast_string(eval->env, buffer, true);

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_ast };
  return 1;
}

void glms_file_constructor(GLMSEval *eval, GLMSStack *stack,
                                  GLMSASTBuffer *args, GLMSAST *self) {

  if (!self) return;
  self->type = GLMS_AST_TYPE_STRUCT;
  self->constructor = glms_file_constructor;
  //self->to_string = glms_file_to_string;
  glms_ast_register_function(eval->env, self, "open", glms_file_fptr_open);
  glms_ast_register_function(eval->env, self, "close", glms_file_fptr_close);
  glms_ast_register_function(eval->env, self, "write", glms_file_fptr_write);
  glms_ast_register_function(eval->env, self, "readLines", glms_file_fptr_read_lines);
  glms_ast_register_function(eval->env, self, "read", glms_file_fptr_read);

  glms_env_register_function_signature(
    eval->env,
    self,
    "readLines",
    (GLMSFunctionSignature){
      .return_type = (GLMSType){ GLMS_AST_TYPE_ITERATOR },
      .args_length = 0
    }
  );

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
