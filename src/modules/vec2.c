#include "glms/ast.h"
#include "vec3/vec3.h"
#include <glms/eval.h>
#include <glms/env.h>


int glms_struct_vec2_swizzle(GLMSEval *eval, GLMSStack *stack, GLMSAST *ast,
                             GLMSAST *accessor, GLMSAST *out) {

  const char *id = glms_ast_get_string_value(accessor);
  if (!id)
    return 0;

  int idx = 0;
  Vector2 v2 = ast->as.v2;

  if (strlen(id) > 1) {
    if (strcmp(id, "xy") == 0) {
      *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC2, .as.v2 = v2};
      return 1;
    } else if (strcmp(id, "yx")) {
      *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC2, .as.v2 = VEC2(v2.y, v2.x)};
      return 1;
    } else {
      GLMS_WARNING_RETURN(0, stderr, "Unsupported swizzle %s\n", id);
    }
  } else {

    idx = ((int)id[0]) - 120;

    switch (id[0]) {
	case 'x': idx = 0; break;
	case 'y': idx = 1; break;
    }
  }

  GLMS_WARNING_RETURN(0, stderr, "Invalid swizzle `%s`\n", id);
  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = idx == 0 ? v2.x : v2.y};
  return 1;
}

int glms_struct_vec2_op_overload_mul(GLMSEval *eval, GLMSStack *stack,
                                     GLMSAST *left, GLMSAST *right,
                                     GLMSAST *out) {


  
  if (!glms_ast_is_vector(left) && !glms_ast_is_vector(right))
    return 0;

  Vector2 v = VEC2(0, 0);


  if (left->type == GLMS_AST_TYPE_VEC2 && right->type == GLMS_AST_TYPE_NUMBER) {
    v = left->as.v2;
    v = vector2_scale(v, right->as.number.value);
  } else if (left->type == GLMS_AST_TYPE_NUMBER &&
             right->type == GLMS_AST_TYPE_VEC2) {
    v = right->as.v2;
    v = vector2_scale(v, left->as.number.value);
  } else if (left->type == GLMS_AST_TYPE_VEC2 &&
             right->type == GLMS_AST_TYPE_VEC2) {
    v = vector2_mul(left->as.v2, right->as.v2);
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC2, .as.v2 = v};
  return 1;
}

int glms_struct_vec2_op_overload_add(GLMSEval *eval, GLMSStack *stack,
                                     GLMSAST *left, GLMSAST *right,
                                     GLMSAST *out) {


  
  if (!glms_ast_is_vector(left) && !glms_ast_is_vector(right))
    return 0;

  Vector2 v = VEC2(0, 0);


  if (left->type == GLMS_AST_TYPE_VEC2 && right->type == GLMS_AST_TYPE_NUMBER) {
    v = left->as.v2;
    v.x += glms_ast_number(*right);
    v.y += glms_ast_number(*right);
  } else if (left->type == GLMS_AST_TYPE_NUMBER &&
             right->type == GLMS_AST_TYPE_VEC2) {
    v = right->as.v2;
    v.x += glms_ast_number(*left);
    v.y += glms_ast_number(*left);
  } else if (left->type == GLMS_AST_TYPE_VEC2 &&
             right->type == GLMS_AST_TYPE_VEC2) {
    v = vector2_add(left->as.v2, right->as.v2);
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC2, .as.v2 = v};
  return 1;
}

int glms_struct_vec2_op_overload_div(GLMSEval *eval, GLMSStack *stack,
                                     GLMSAST *left, GLMSAST *right,
                                     GLMSAST *out) {

  if (!glms_ast_is_vector(left) && !glms_ast_is_vector(right))
    return 0;

  Vector2 v = VEC2(0, 0);

  if (left->type == GLMS_AST_TYPE_VEC2 && right->type == GLMS_AST_TYPE_NUMBER) {
    v = left->as.v2;
    v = vector2_scale(v, 1.0f / right->as.number.value);
  } else if (left->type == GLMS_AST_TYPE_NUMBER &&
             right->type == GLMS_AST_TYPE_VEC2) {
    v = right->as.v2;
    v = vector2_scale(v, 1.0f / left->as.number.value);
  } else if (left->type == GLMS_AST_TYPE_VEC2 &&
             right->type == GLMS_AST_TYPE_VEC2) {
    v.x = left->as.v2.x / right->as.v2.x;
    v.y = left->as.v2.y / right->as.v2.y;
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC2, .as.v2 = v};

  return 1;
}

int glms_struct_vec2_op_overload_sub(GLMSEval *eval, GLMSStack *stack,
                                     GLMSAST *left, GLMSAST *right,
                                     GLMSAST *out) {

  if (!glms_ast_is_vector(left) && !glms_ast_is_vector(right))
    return 0;

  Vector2 v = VEC2(0, 0);

  if (left->type == GLMS_AST_TYPE_VEC2 && right->type == GLMS_AST_TYPE_NUMBER) {
    v = left->as.v2;
    float f = glms_ast_number(*right);
    v.x -= f; 
    v.y -= f; 
  } else if (left->type == GLMS_AST_TYPE_NUMBER &&
             right->type == GLMS_AST_TYPE_VEC2) {
    float f = glms_ast_number(*left);
    v = right->as.v2;

    v.x -= f; 
    v.y -= f; 
  } else if (left->type == GLMS_AST_TYPE_VEC2 &&
             right->type == GLMS_AST_TYPE_VEC2) {
    v = vector2_sub(left->as.v2, right->as.v2);
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC2, .as.v2 = v};

  return 1;
}


char *glms_struct_vec2_to_string(GLMSAST *ast, GLMSAllocator alloc, GLMSEnv* env) {
  Vector2 v = ast->as.v2;

  char tmp[256];
  sprintf(tmp, "VEC2(%1.6f, %1.6f)", v.x, v.y);

  return alloc.strdup(alloc.user_ptr, tmp);
}

void glms_struct_vec2_constructor(GLMSEval *eval, GLMSStack *stack,
                                  GLMSASTBuffer *args, GLMSAST *self) {

  if (!self)
    return;

  GLMSAST *ast = self;
  ast->type = GLMS_AST_TYPE_VEC2;
  ast->swizzle = glms_struct_vec2_swizzle;
  ast->constructor = glms_struct_vec2_constructor;
  ast->to_string = glms_struct_vec2_to_string;
  glms_ast_register_operator_overload(eval->env, ast, GLMS_TOKEN_TYPE_MUL,
                                      glms_struct_vec2_op_overload_mul);
  glms_ast_register_operator_overload(eval->env, ast, GLMS_TOKEN_TYPE_DIV,
                                      glms_struct_vec2_op_overload_div);
  glms_ast_register_operator_overload(eval->env, ast, GLMS_TOKEN_TYPE_SUB,
                                      glms_struct_vec2_op_overload_sub);

    glms_ast_register_operator_overload(eval->env, ast, GLMS_TOKEN_TYPE_ADD,
                                      glms_struct_vec2_op_overload_add);


  if (!args)
    return;

  if (args->length >= 2) {
    float x = (glms_eval(eval, args->items[0], stack)).as.number.value;
    float y = (glms_eval(eval, args->items[1], stack)).as.number.value;

    ast->as.v2 = VEC2(x, y);
  } else if (args->length == 1) {
    float x = (glms_eval(eval, args->items[0], stack)).as.number.value;
    ast->as.v2 = VEC2(x, x);
  }
}

void glms_struct_vec2(GLMSEnv *env) {
  GLMSAST* t = glms_env_new_ast(env, GLMS_AST_TYPE_VEC2, false);
  glms_env_register_type(
      env, "vec2", t,
      glms_struct_vec2_constructor, glms_struct_vec2_swizzle, 0, 0);

  glms_env_register_type(
      env, GLMS_AST_TYPE_STR[GLMS_AST_TYPE_VEC2], t,
      glms_struct_vec2_constructor, glms_struct_vec2_swizzle, 0, 0);
  //  glms_env_register_struct(env, "vec2", (GLMSAST*[]){
  //    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "x"),
  //    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "y"),
  //    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "z")
  //  }, 3);
}
