#include "cglm/mat4.h"
#include "cglm/cam.h"
#include "cglm/struct/mat4.h"
#include "cglm/struct/vec4.h"
#include "cglm/types-struct.h"
#include "glms/allocator.h"
#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/env.h"
#include "glms/eval.h"
#include "glms/token.h"
#include <glms/modules/mat4.h>
#include <text/text.h>

// typedef char* (*GLMSASTToMat4)(struct GLMS_AST_STRUCT *ast, GLMSAllocator alloc);

char *glms_mat4_to_string(GLMSAST *ast, GLMSAllocator alloc, GLMSEnv* env) {
  char* s = 0;

  alloc.strcat(alloc.user_ptr, &s, "{\n");
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {

      char tmp[16];
      sprintf(tmp, "%1.6f", ast->as.m4.col[i].raw[j]);
      alloc.strcat(alloc.user_ptr, &s, tmp);

      if (j < 4-1) {
	alloc.strcat(alloc.user_ptr, &s, ", ");
      }
    }

    if (i < 4-1) {
      alloc.strcat(alloc.user_ptr, &s, "\n");
    }
  }
  alloc.strcat(alloc.user_ptr, &s, "\n}");

  return s;
}

int glms_mat4_type_op_overload_mul(GLMSEval *eval, GLMSStack *stack,
                                     GLMSAST *left, GLMSAST *right,
                                     GLMSAST *out) {


  if (left->type == GLMS_AST_TYPE_MAT4 && right->type == GLMS_AST_TYPE_MAT4) {
    *out = (GLMSAST){ .type = GLMS_AST_TYPE_MAT4, .as.m4 = glms_mat4_mul(left->as.m4, right->as.m4) };
    return 1;
  }
  else if (left->type == GLMS_AST_TYPE_MAT4 && right->type == GLMS_AST_TYPE_VEC4) {
    Vector4 v = right->as.v4;
    vec4s r = glms_mat4_mulv(left->as.m4, (vec4s){ v.x, v.y, v.z, v.w });
    *out = (GLMSAST){ .type = GLMS_AST_TYPE_VEC4, .as.v4 = VEC4(r.x, r.y, r.z, r.w) };
    return 1;
  } else if (left->type == GLMS_AST_TYPE_MAT4 && right->type == GLMS_AST_TYPE_VEC3) {
    Vector3 v = right->as.v3;
    vec4s r = glms_mat4_mulv(left->as.m4, (vec4s){ v.x, v.y, v.z, 1.0f });
    *out = (GLMSAST){ .type = GLMS_AST_TYPE_VEC4, .as.v4 = VEC4(r.x, r.y, r.z, r.w) };
    return 1;
  } else if (right->type == GLMS_AST_TYPE_MAT4 && left->type == GLMS_AST_TYPE_VEC4) {
    Vector4 v = left->as.v4;
    vec4s r = glms_mat4_mulv(right->as.m4, (vec4s){ v.x, v.y, v.z, v.w });
    *out = (GLMSAST){ .type = GLMS_AST_TYPE_VEC4, .as.v4 = VEC4(r.x, r.y, r.z, r.w) };
    return 1;
  } else if (right->type == GLMS_AST_TYPE_MAT4 && left->type == GLMS_AST_TYPE_VEC3) {
    Vector3 v = left->as.v3;
    vec4s r = glms_mat4_mulv(right->as.m4, (vec4s){ v.x, v.y, v.z, 1.0f });
    *out = (GLMSAST){ .type = GLMS_AST_TYPE_VEC4, .as.v4 = VEC4(r.x, r.y, r.z, r.w) };
    return 1;
  }


  //glm_mat4_zero(result.as.m4);


  // glm_mat4_mul(left->as.m4, right->as.m4, out->as.m4);

  return 0;
}

void glms_mat4_constructor(GLMSEval *eval, GLMSStack *stack,
                                  GLMSASTBuffer *args, GLMSAST *self) {

  if (!self) return;
  self->type = GLMS_AST_TYPE_MAT4;
  self->constructor = glms_mat4_constructor;
  self->to_string = glms_mat4_to_string;

  glms_ast_register_operator_overload(eval->env, self, GLMS_TOKEN_TYPE_MUL, glms_mat4_type_op_overload_mul);
}

void glms_mat4_type(GLMSEnv *env) {
  GLMSAST* m4ast = glms_env_new_ast(env, GLMS_AST_TYPE_MAT4, false);
  m4ast->constructor = glms_mat4_constructor;
  glms_env_register_type(env, "mat4", m4ast, glms_mat4_constructor, 0, glms_mat4_to_string, 0);
  glms_env_register_type(env, GLMS_AST_TYPE_STR[GLMS_AST_TYPE_MAT4], m4ast, glms_mat4_constructor, 0, glms_mat4_to_string, 0);
}
