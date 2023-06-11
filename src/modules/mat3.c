#include "cglm/mat3.h"
#include "cglm/cam.h"
#include "cglm/struct/mat3.h"
#include "cglm/struct/vec3.h"
#include "cglm/types-struct.h"
#include "glms/allocator.h"
#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/env.h"
#include "glms/eval.h"
#include "glms/token.h"
#include <glms/modules/mat3.h>
#include <text/text.h>

// typedef char* (*GLMSASTToMat3)(struct GLMS_AST_STRUCT *ast, GLMSAllocator alloc);

char *glms_mat3_to_string(GLMSAST *ast, GLMSAllocator alloc, GLMSEnv* env) {
  char* s = 0;

  alloc.strcat(alloc.user_ptr, &s, "{\n");
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {

      char tmp[16];
      sprintf(tmp, "%1.6f", ast->as.m3.col[i].raw[j]);
      alloc.strcat(alloc.user_ptr, &s, tmp);

      if (j < 3-1) {
	alloc.strcat(alloc.user_ptr, &s, ", ");
      }
    }

    if (i < 3-1) {
      alloc.strcat(alloc.user_ptr, &s, "\n");
    }
  }
  alloc.strcat(alloc.user_ptr, &s, "\n}");

  return s;
}

int glms_mat3_type_op_overload_mul(GLMSEval *eval, GLMSStack *stack,
                                     GLMSAST *left, GLMSAST *right,
                                     GLMSAST *out) {


  if (left->type == GLMS_AST_TYPE_MAT3 && right->type == GLMS_AST_TYPE_MAT3) {
    *out = (GLMSAST){ .type = GLMS_AST_TYPE_MAT3, .as.m3 = glms_mat3_mul(left->as.m3, right->as.m3) };
    return 1;
  }
  else if (left->type == GLMS_AST_TYPE_MAT3 && right->type == GLMS_AST_TYPE_VEC3) {
    Vector3 v = right->as.v3;
    vec3s r = glms_mat3_mulv(left->as.m3, (vec3s){ v.x, v.y, v.z });
    *out = (GLMSAST){ .type = GLMS_AST_TYPE_VEC3, .as.v3 = VEC3(r.x, r.y, r.z) };
    return 1;
  } else if (left->type == GLMS_AST_TYPE_MAT3 && right->type == GLMS_AST_TYPE_VEC3) {
    Vector3 v = right->as.v3;
    vec3s r = glms_mat3_mulv(left->as.m3, (vec3s){ v.x, v.y, v.z });
    *out = (GLMSAST){ .type = GLMS_AST_TYPE_VEC3, .as.v3 = VEC3(r.x, r.y, r.z) };
    return 1;
  } else if (right->type == GLMS_AST_TYPE_MAT3 && left->type == GLMS_AST_TYPE_VEC3) {
    Vector3 v = left->as.v3;
    vec3s r = glms_mat3_mulv(right->as.m3, (vec3s){ v.x, v.y, v.z });
    *out = (GLMSAST){ .type = GLMS_AST_TYPE_VEC3, .as.v3 = VEC3(r.x, r.y, r.z) };
    return 1;
  } else if (right->type == GLMS_AST_TYPE_MAT3 && left->type == GLMS_AST_TYPE_VEC3) {
    Vector3 v = left->as.v3;
    vec3s r = glms_mat3_mulv(right->as.m3, (vec3s){ v.x, v.y, v.z });
    *out = (GLMSAST){ .type = GLMS_AST_TYPE_VEC3, .as.v3 = VEC3(r.x, r.y, r.z) };
    return 1;
  }


  //glm_mat3_zero(result.as.m3);


  // glm_mat3_mul(left->as.m3, right->as.m3, out->as.m3);

  return 0;
}

void glms_mat3_constructor(GLMSEval *eval, GLMSStack *stack,
                                  GLMSASTBuffer *args, GLMSAST *self) {

  if (!self) return;
  self->type = GLMS_AST_TYPE_MAT3;
  self->constructor = glms_mat3_constructor;
  self->to_string = glms_mat3_to_string;

  glms_ast_register_operator_overload(eval->env, self, GLMS_TOKEN_TYPE_MUL, glms_mat3_type_op_overload_mul);
}

void glms_mat3_type(GLMSEnv *env) {
  GLMSAST* m3ast = glms_env_new_ast(env, GLMS_AST_TYPE_MAT3, false);
  m3ast->constructor = glms_mat3_constructor;
  glms_env_register_type(env, "mat3", m3ast, glms_mat3_constructor, 0, glms_mat3_to_string, 0);
  glms_env_register_type(env, GLMS_AST_TYPE_STR[GLMS_AST_TYPE_MAT3], m3ast, glms_mat3_constructor, 0, glms_mat3_to_string, 0);
}
