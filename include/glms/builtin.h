#ifndef GLMS_BUILTIN_H
#define GLMS_BUILTIN_H
#include <glms/glms.h>


void glms_builtin_init(GLMSEnv* env);

GLMSAST *glms_fptr_print(GLMSEval *eval, GLMSAST *ast,
                               GLMSASTList *args, GLMSStack *stack);


GLMSAST *glms_fptr_dot(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                       GLMSStack *stack);

GLMSAST *glms_fptr_distance(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                       GLMSStack *stack);

GLMSAST *glms_fptr_cross(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                         GLMSStack *stack);

GLMSAST *glms_fptr_normalize(GLMSEval *eval, GLMSAST *ast,
                             GLMSASTList *args, GLMSStack *stack);

GLMSAST *glms_fptr_length(GLMSEval *eval, GLMSAST *ast,
                          GLMSASTList *args, GLMSStack *stack);

GLMSAST *glms_fptr_cos(GLMSEval *eval, GLMSAST *ast,
                       GLMSASTList *args, GLMSStack *stack);

GLMSAST *glms_fptr_sin(GLMSEval *eval, GLMSAST *ast,
                               GLMSASTList *args, GLMSStack *stack);

GLMSAST *glms_fptr_lerp(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                        GLMSStack *stack);

GLMSAST *glms_fptr_clamp(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                        GLMSStack *stack);

GLMSAST *glms_fptr_random(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                          GLMSStack *stack);

GLMSAST *glms_struct_vec3_constructor(GLMSEval *eval, GLMSStack *stack,
                                      GLMSASTList *args);


const char *glms_struct_vec3_to_string(GLMSAST *ast);

#endif
