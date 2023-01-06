#ifndef GLMS_BUILTIN_H
#define GLMS_BUILTIN_H
#include <glms/glms.h>
#include <glms/type.h>

#include "glms/ast.h"

void glms_builtin_init(GLMSEnv *env);

int glms_fptr_dot(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out);

int glms_fptr_distance(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                       GLMSStack *stack, GLMSAST *out);

int glms_fptr_cross(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                    GLMSStack *stack, GLMSAST *out);

int glms_fptr_normalize(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out);

int glms_fptr_length(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                     GLMSStack *stack, GLMSAST *out);

int glms_fptr_cos(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out);

int glms_fptr_sin(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out);

int glms_fptr_tan(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out);

int glms_fptr_atan(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                   GLMSStack *stack, GLMSAST *out);

int glms_fptr_fract(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                    GLMSStack *stack, GLMSAST *out);

int glms_fptr_abs(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out);

int glms_fptr_pow(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out);

int glms_fptr_log(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out);

int glms_fptr_log10(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                    GLMSStack *stack, GLMSAST *out);

int glms_fptr_lerp(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                   GLMSStack *stack, GLMSAST *out);

int glms_fptr_clamp(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                    GLMSStack *stack, GLMSAST *out);

int glms_fptr_trace(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                    GLMSStack *stack, GLMSAST *out);

int glms_fptr_random(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                     GLMSStack *stack, GLMSAST *out);

#endif
