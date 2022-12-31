#include "glms/ast.h"
#include <glms/eval.h>
#include <glms/env.h>


int glms_struct_vec3_swizzle(GLMSEval *eval, GLMSStack *stack, GLMSAST *ast,
                             GLMSAST *accessor, GLMSAST *out) {

  const char *id = glms_ast_get_string_value(accessor);
  if (!id)
    return 0;

  if (strlen(id) > 1) {
    GLMS_WARNING_RETURN(0, stderr, "Invalid swizzle `%s`\n", id);
  }

  int idx = ((int)id[0]) - 120;

  if (idx < 0 || idx >= 3) {
    GLMS_WARNING_RETURN(0, stderr, "Invalid swizzle `%s`\n", id);
  }

  float v = vector3_get_component(ast->as.v3, idx);

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = v};
  return 1;
}

int glms_struct_vec3_op_overload_mul(GLMSEval *eval, GLMSStack *stack,
                                     GLMSAST *left, GLMSAST *right,
                                     GLMSAST *out) {


  
  if (!glms_ast_is_vector(left) && !glms_ast_is_vector(right))
    return 0;

  Vector3 v = VEC31(0);


  if (left->type == GLMS_AST_TYPE_VEC3 && right->type == GLMS_AST_TYPE_NUMBER) {
    v = left->as.v3;
    v = vector3_scale(v, right->as.number.value);
  } else if (left->type == GLMS_AST_TYPE_NUMBER &&
             right->type == GLMS_AST_TYPE_VEC3) {
    v = right->as.v3;
    v = vector3_scale(v, left->as.number.value);
  } else if (left->type == GLMS_AST_TYPE_VEC3 &&
             right->type == GLMS_AST_TYPE_VEC3) {
    v = vector3_mul(left->as.v3, right->as.v3);
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC3, .as.v3 = v};
  return 1;
}
int glms_struct_vec3_op_overload_div(GLMSEval *eval, GLMSStack *stack,
                                     GLMSAST *left, GLMSAST *right,
                                     GLMSAST *out) {

  if (!glms_ast_is_vector(left) && !glms_ast_is_vector(right))
    return 0;

  Vector3 v = VEC31(0);

  if (left->type == GLMS_AST_TYPE_VEC3 && right->type == GLMS_AST_TYPE_NUMBER) {
    v = left->as.v3;
    v = vector3_scale(v, 1.0f / right->as.number.value);
  } else if (left->type == GLMS_AST_TYPE_NUMBER &&
             right->type == GLMS_AST_TYPE_VEC3) {
    v = right->as.v3;
    v = vector3_scale(v, 1.0f / left->as.number.value);
  } else if (left->type == GLMS_AST_TYPE_VEC3 &&
             right->type == GLMS_AST_TYPE_VEC3) {
    v.x = left->as.v3.x / right->as.v3.x;
    v.y = left->as.v3.y / right->as.v3.y;
    v.z = left->as.v3.z / right->as.v3.z;
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC3, .as.v3 = v};

  return 1;
}

char *glms_struct_vec3_to_string(GLMSAST *ast, GLMSAllocator alloc) {
  Vector3 v = ast->as.v3;

  char tmp[256];
  sprintf(tmp, "VEC3(%1.6f, %1.6f, %1.6f)", v.x, v.y, v.z);

  return alloc.strdup(alloc.user_ptr, tmp);
}

void glms_struct_vec3_constructor(GLMSEval *eval, GLMSStack *stack,
                                  GLMSASTBuffer *args, GLMSAST *self) {

  if (!self)
    return;

  GLMSAST *ast = self;
  ast->type = GLMS_AST_TYPE_VEC3;
  ast->swizzle = glms_struct_vec3_swizzle;
  ast->constructor = glms_struct_vec3_constructor;
  ast->to_string = glms_struct_vec3_to_string;
  glms_ast_register_operator_overload(eval->env, ast, GLMS_TOKEN_TYPE_MUL,
                                      glms_struct_vec3_op_overload_mul);
  glms_ast_register_operator_overload(eval->env, ast, GLMS_TOKEN_TYPE_DIV,
                                      glms_struct_vec3_op_overload_div);

  if (!args)
    return;

  if (args->length >= 3) {
    float x = (glms_eval(eval, args->items[0], stack)).as.number.value;
    float y = (glms_eval(eval, args->items[1], stack)).as.number.value;
    float z = (glms_eval(eval, args->items[2], stack)).as.number.value;

    ast->as.v3 = VEC3(x, y, z);
  } else if (args->length == 1) {
    float x = (glms_eval(eval, args->items[0], stack)).as.number.value;
    ast->as.v3 = VEC31(x);
  }
}

void glms_struct_vec3(GLMSEnv *env) {
  glms_env_register_type(
      env, "vec3", glms_env_new_ast(env, GLMS_AST_TYPE_VEC3, false),
      glms_struct_vec3_constructor, glms_struct_vec3_swizzle, 0, 0);
  //  glms_env_register_struct(env, "vec3", (GLMSAST*[]){
  //    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "x"),
  //    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "y"),
  //    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "z")
  //  }, 3);
}
