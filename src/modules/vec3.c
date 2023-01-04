#include "glms/ast.h"
#include "vec3/vec3.h"
#include <glms/eval.h>
#include <glms/env.h>


int glms_struct_vec3_swizzle(GLMSEval *eval, GLMSStack *stack, GLMSAST *ast,
                             GLMSAST *accessor, GLMSAST *out) {

  const char *id = glms_ast_get_string_value(accessor);
  if (!id)
    return 0;

  int idx = 0;
  Vector3 v3 = ast->as.v3;

  if (strlen(id) > 1) {
    if (strcmp(id, "xyz") == 0) {
      *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC3, .as.v3 = v3};
      return 1;
    } else {
      GLMS_WARNING_RETURN(0, stderr, "Unsupported swizzle %s\n", id);
    }
  } else {

    idx = ((int)id[0]) - 120;

    switch (id[0]) {
	case 'r': idx = 0; break;
	case 'g': idx = 1; break;
	case 'b': idx = 2; break;
	default: {}; break;
    }
  }

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

int glms_struct_vec3_op_overload_add(GLMSEval *eval, GLMSStack *stack,
                                     GLMSAST *left, GLMSAST *right,
                                     GLMSAST *out) {


  
  if (!glms_ast_is_vector(left) && !glms_ast_is_vector(right))
    return 0;

  Vector3 v = VEC31(0);


  if (left->type == GLMS_AST_TYPE_VEC3 && right->type == GLMS_AST_TYPE_NUMBER) {
    v = left->as.v3;
    v.x += glms_ast_number(*right);
    v.y += glms_ast_number(*right);
    v.z += glms_ast_number(*right);
  } else if (left->type == GLMS_AST_TYPE_NUMBER &&
             right->type == GLMS_AST_TYPE_VEC3) {
    v = right->as.v3;
    v.x += glms_ast_number(*left);
    v.y += glms_ast_number(*left);
    v.z += glms_ast_number(*left);
  } else if (left->type == GLMS_AST_TYPE_VEC3 &&
             right->type == GLMS_AST_TYPE_VEC3) {
    v = vector3_add(left->as.v3, right->as.v3);
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

int glms_struct_vec3_op_overload_sub(GLMSEval *eval, GLMSStack *stack,
                                     GLMSAST *left, GLMSAST *right,
                                     GLMSAST *out) {

  if (!glms_ast_is_vector(left) && !glms_ast_is_vector(right))
    return 0;

  Vector3 v = VEC31(0);

  if (left->type == GLMS_AST_TYPE_VEC3 && right->type == GLMS_AST_TYPE_NUMBER) {
    v = left->as.v3;
    float f = glms_ast_number(*right);
    v.x -= f; 
    v.y -= f; 
    v.z -= f; 
  } else if (left->type == GLMS_AST_TYPE_NUMBER &&
             right->type == GLMS_AST_TYPE_VEC3) {
    float f = glms_ast_number(*left);
    v = right->as.v3;

    v.x -= f; 
    v.y -= f; 
    v.z -= f; 
  } else if (left->type == GLMS_AST_TYPE_VEC3 &&
             right->type == GLMS_AST_TYPE_VEC3) {
    v = vector3_sub(left->as.v3, right->as.v3);
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC3, .as.v3 = v};

  return 1;
}

int glms_struct_vec3_func_overload_mix(struct GLMS_EVAL_STRUCT* eval, struct GLMS_AST_STRUCT* ast, struct GLMS_BUFFER_GLMSAST* args, struct GLMS_STACK_STRUCT* stack, struct GLMS_AST_STRUCT* out) {


  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_VEC3, GLMS_AST_TYPE_VEC3, GLMS_AST_TYPE_NUMBER }, 3, args)) return 0;


  Vector3 v1 = args->items[0].as.v3;
  Vector3 v2 = args->items[1].as.v3;
  float scale = glms_ast_number(args->items[2]);

  Vector3 v = vector3_lerp_factor(v1, v2, scale);

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_VEC3, .as.v3 = v};

  return 1;
}

char *glms_struct_vec3_to_string(GLMSAST *ast, GLMSAllocator alloc, GLMSEnv* env) {
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
  glms_ast_register_operator_overload(eval->env, ast, GLMS_TOKEN_TYPE_SUB,
                                      glms_struct_vec3_op_overload_sub);

    glms_ast_register_operator_overload(eval->env, ast, GLMS_TOKEN_TYPE_ADD,
                                      glms_struct_vec3_op_overload_add);

  glms_ast_register_func_overload(eval->env, ast, "mix", glms_struct_vec3_func_overload_mix);
  glms_ast_register_func_overload(eval->env, ast, "lerp", glms_struct_vec3_func_overload_mix);

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
  GLMSAST* t = glms_env_new_ast(env, GLMS_AST_TYPE_VEC3, false);
  glms_env_register_type(
      env, "vec3", t,
      glms_struct_vec3_constructor, glms_struct_vec3_swizzle, 0, 0);

  glms_env_register_type(
      env, GLMS_AST_TYPE_STR[GLMS_AST_TYPE_VEC3], t,
      glms_struct_vec3_constructor, glms_struct_vec3_swizzle, 0, 0);
  //  glms_env_register_struct(env, "vec3", (GLMSAST*[]){
  //    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "x"),
  //    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "y"),
  //    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "z")
  //  }, 3);
}
