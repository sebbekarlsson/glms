#ifndef GLMS_BUILTIN_H
#define GLMS_BUILTIN_H
#include "glms/ast.h"
#include <glms/glms.h>
#include <glms/type.h>


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

GLMSAST *glms_fptr_sin(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                       GLMSStack *stack);

GLMSAST *glms_fptr_tan(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                       GLMSStack *stack);

GLMSAST *glms_fptr_atan(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                        GLMSStack *stack);

GLMSAST *glms_fptr_fract(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                         GLMSStack *stack);

GLMSAST *glms_fptr_abs(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                       GLMSStack *stack);

GLMSAST *glms_fptr_lerp(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                        GLMSStack *stack);

GLMSAST *glms_fptr_clamp(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                         GLMSStack *stack);

GLMSAST *glms_fptr_keep(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                        GLMSStack *stack);

GLMSAST *glms_fptr_trace(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                        GLMSStack *stack);

GLMSAST *glms_fptr_random(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                          GLMSStack *stack);

GLMSAST *glms_struct_vec3_constructor(GLMSEval *eval, GLMSStack *stack,
                                      GLMSASTList *args, GLMSAST* self);


const char *glms_struct_vec3_to_string(GLMSAST *ast);
const char *glms_struct_vec4_to_string(GLMSAST *ast);


bool glms_fptr_expect_args(
			   GLMSEval* eval,
			   GLMSStack* stack,
			   GLMSASTList* in_args,
			   GLMSType* types,
			   int nr_types
			   );

#endif
