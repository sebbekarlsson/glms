#include "cglm/struct/vec4.h"
#include "glms/allocator.h"
#include "glms/ast.h"
#include <glms/env.h>
#include <glms/eval.h>


char *glms_struct_vec4_to_string(GLMSAST *ast, GLMSAllocator alloc, GLMSEnv* env) {
  Vector3 v = ast->as.v3;

  char tmp[256];
  sprintf(tmp, "VEC4(%1.6f, %1.6f, %1.6f, %1.6f)", v.x, v.y, v.z, v.w);

  return alloc.strdup(alloc.user_ptr, tmp);
}

int glms_struct_vec4_swizzle(GLMSEval *eval, GLMSStack *stack, GLMSAST *ast,
                             GLMSAST *accessor, GLMSAST *out) {
  const char *id = glms_ast_get_string_value(accessor);
  if (!id)
    return 0;

  if (strlen(id) > 1) {
    GLMS_WARNING_RETURN(0, stderr, "Invalid swizzle `%s`\n", id);
  }

  int idx = ((int)id[0]) - 120;

  if (idx < 0 || idx >= 4) {
    GLMS_WARNING_RETURN(0, stderr, "Invalid swizzle `%s`\n", id);
  }

  float* values = (float*)&ast->as.v4;

  float v = values[idx];

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = v};
  return 1;
}

int glms_struct_vec4_op_overload_mul(GLMSEval *eval, GLMSStack *stack,
                                     GLMSAST *left, GLMSAST *right,
                                     GLMSAST *out) {


  
  if (!glms_ast_is_vector(left) && !glms_ast_is_vector(right))
    return 0;

  Vector4 v = VEC41(0);


  if (left->type == GLMS_AST_TYPE_VEC4 && right->type == GLMS_AST_TYPE_NUMBER) {
    v = left->as.v4;
    v.x *=  glms_ast_number(*right); 
    v.y *=  glms_ast_number(*right); 
    v.z *=  glms_ast_number(*right); 
    v.w *=  glms_ast_number(*right); 
  } else if (left->type == GLMS_AST_TYPE_NUMBER &&
             right->type == GLMS_AST_TYPE_VEC4) {
    v = right->as.v4;
    v.x *=  glms_ast_number(*left); 
    v.y *=  glms_ast_number(*left); 
    v.z *=  glms_ast_number(*left); 
    v.w *=  glms_ast_number(*left); 
  } else if (left->type == GLMS_AST_TYPE_VEC4 &&
             right->type == GLMS_AST_TYPE_VEC4) {
    v.x = left->as.v4.x * right->as.v4.x; 
    v.y = left->as.v4.y * right->as.v4.y; 
    v.z = left->as.v4.z * right->as.v4.z; 
    v.w = left->as.v4.w * right->as.v4.w; 
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC4, .as.v4 = v};
  return 1;
}


void glms_struct_vec4_constructor(GLMSEval *eval, GLMSStack *stack,
                                      GLMSASTBuffer *args, GLMSAST* self) {
  if (!self) return;
  GLMSAST *ast = self ;//? self : glms_env_new_ast(eval->env, GLMS_AST_TYPE_VEC4, true);

  ast->type = GLMS_AST_TYPE_VEC4;
  ast->swizzle = glms_struct_vec4_swizzle;
  ast->constructor = glms_struct_vec4_constructor;
  ast->to_string = glms_struct_vec4_to_string;

  glms_ast_register_operator_overload(eval->env, ast, GLMS_TOKEN_TYPE_MUL,
                                      glms_struct_vec4_op_overload_mul);

  if (!args)
    return;

  if (args->length == 2) {
    Vector3 xyz = args->items[0].as.v3;
    float w = glms_ast_number(args->items[1]); 
    ast->as.v4 = VEC4(xyz.x, xyz.y, xyz.z, w);
  } else if (args->length >= 4) {
    float x = glms_ast_number(args->items[0]);
    float y = glms_ast_number(args->items[1]);
    float z = glms_ast_number(args->items[2]);
    float w = glms_ast_number(args->items[3]);

    ast->as.v4 = VEC4(x, y, z, w);
  } else if (args->length == 1) {
    float x = (glms_eval(eval, args->items[0], stack)).as.number.value;
    ast->as.v4 = VEC41(x);
  }

}

void glms_struct_vec4(GLMSEnv *env) {
  GLMSAST* typeast = glms_env_new_ast(env, GLMS_AST_TYPE_VEC4, false);
  glms_env_register_type(env, "vec4", typeast,
                         glms_struct_vec4_constructor, glms_struct_vec4_swizzle,
                         glms_struct_vec4_to_string, 0);

  glms_env_register_type(env, GLMS_AST_TYPE_STR[GLMS_AST_TYPE_VEC4], typeast,
                         glms_struct_vec4_constructor, glms_struct_vec4_swizzle,
                         glms_struct_vec4_to_string, 0);
}
