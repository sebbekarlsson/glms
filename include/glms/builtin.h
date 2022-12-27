#ifndef GLMS_BUILTIN_H
#define GLMS_BUILTIN_H
#include <glms/glms.h>


void glms_builtin_init(GLMSEnv* env);

GLMSAST *glms_fptr_print(GLMSEval *eval, GLMSAST *ast,
                               GLMSASTList *args, GLMSStack *stack);


GLMSAST *glms_fptr_dot(GLMSEval *eval, GLMSAST *ast,
                             GLMSASTList *args, GLMSStack *stack);

GLMSAST *glms_fptr_length(GLMSEval *eval, GLMSAST *ast,
                          GLMSASTList *args, GLMSStack *stack);

GLMSAST *glms_fptr_cos(GLMSEval *eval, GLMSAST *ast,
                       GLMSASTList *args, GLMSStack *stack);

GLMSAST *glms_fptr_sin(GLMSEval *eval, GLMSAST *ast,
                               GLMSASTList *args, GLMSStack *stack);

GLMSAST *glms_fptr_lerp(GLMSEval *eval, GLMSAST *ast,
                               GLMSASTList *args, GLMSStack *stack);
#endif
