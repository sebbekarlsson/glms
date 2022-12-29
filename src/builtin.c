#include "glms/ast_type.h"
#include "glms/eval.h"
#include "glms/stack.h"
#include "glms/token.h"
#include "text/text.h"
#include <glms/ast.h>
#include <glms/env.h>
#include <glms/macros.h>
#include <vec3/vec3.h>
#include <glms/builtin.h>
#include <math.h>
#include <mif/utils.h>
#include <stdlib.h>
#include <gimg/gimg.h>
#include <glms/modules/image.h>

GLMSAST *glms_fptr_print(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                         GLMSStack *stack) {
  if (!args)
    return ast;

  for (int64_t i = 0; i < args->length; i++) {
    GLMSAST *arg = glms_eval(eval, args->items[i], stack);
    if (!arg)
      continue;
    switch (arg->type) {
    case GLMS_AST_TYPE_NUMBER: {
      printf("%1.6f\n", arg->as.number.value);
    }; break;
    case GLMS_AST_TYPE_STRING: {
      const char *value = glms_ast_get_string_value(arg);

      if (value) {
        printf("%s\n", value);
      } else {
        printf("(null)\n");
      }
    }; break;
    default: {
      const char* strval = glms_ast_to_string(arg);
      if (strval != 0) {
	printf("%s\n", strval);
      } else {
	printf("%p\n", arg);
      }
    }; break;
    }
  }

  return ast;
}

GLMSAST *glms_fptr_dot(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                       GLMSStack *stack) {

  if (!args)
    return ast;
  if (args->length < 2)
    return ast;

  GLMSAST *a = glms_eval(eval, args->items[0], stack);
  GLMSAST *b = glms_eval(eval, args->items[1], stack);

  if (!glms_ast_is_vector(a) || !glms_ast_is_vector(b))
    GLMS_WARNING_RETURN(ast, stderr, "Not a vector.\n");

  return glms_env_new_ast_number(eval->env, vector3_dot(a->as.v3, b->as.v3), true);
}

GLMSAST *glms_fptr_distance(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                            GLMSStack *stack) {
  if (!args)
    return ast;
  if (args->length < 2)
    return ast;

  GLMSAST *a = glms_eval(eval, args->items[0], stack);
  GLMSAST *b = glms_eval(eval, args->items[1], stack);

  if (!glms_ast_is_vector(a) || !glms_ast_is_vector(b))
    GLMS_WARNING_RETURN(ast, stderr, "Not a vector.\n");

  return glms_env_new_ast_number(eval->env,
                                 vector3_distance3d(a->as.v3, b->as.v3), true);
}

GLMSAST *glms_fptr_cross(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                         GLMSStack *stack) {

  if (args->length < 2)
    return ast;
  GLMSAST *a = glms_eval(eval, args->items[0], stack);
  GLMSAST *b = glms_eval(eval, args->items[1], stack);

  if (!glms_ast_is_vector(a) || !glms_ast_is_vector(b))
    GLMS_WARNING_RETURN(ast, stderr, "Not a vector.\n");

  GLMSAST *result = glms_env_new_ast(eval->env, GLMS_AST_TYPE_VEC3, true);
  result->swizzle = a->swizzle;
  result->to_string = a->to_string;
  result->constructor = a->constructor;
  result->as.v3 = vector3_cross(a->as.v3, b->as.v3);

  return result;
}

GLMSAST *glms_fptr_normalize(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                             GLMSStack *stack) {

  if (args->length <= 0)
    return ast;
  GLMSAST *a = glms_eval(eval, args->items[0], stack);

  if (!glms_ast_is_vector(a))
    GLMS_WARNING_RETURN(ast, stderr, "Not a vector.\n");

  GLMSAST *result = glms_env_new_ast(eval->env, GLMS_AST_TYPE_VEC3, true);
  result->swizzle = a->swizzle;
  result->to_string = a->to_string;
  result->constructor = a->constructor;
  result->as.v3 = vector3_unit(a->as.v3);

  return result;
}

GLMSAST *glms_fptr_length(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                          GLMSStack *stack) {
  if (!args)
    return ast;
  if (args->length <= 0)
    return ast;

  GLMSAST *value = glms_eval(eval, args->items[0], stack);

  if (glms_ast_is_vector(value)) {
    return glms_env_new_ast_number(eval->env, vector3_mag(value->as.v3), true);
  }

  int64_t len = 0;

  switch (value->type) {
  case GLMS_AST_TYPE_STRING: {
    const char *strvalue = glms_ast_get_string_value(value);
    if (!strvalue)
      len = 0;
    len = strlen(strvalue);
  }; break;
  default: {
    len = glms_ast_array_get_length(ast);
  }; break;
  }

  return glms_env_new_ast_number(eval->env, (float)len, true);
}

GLMSAST *glms_fptr_cos(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                       GLMSStack *stack) {

  if (args->length <= 0)
    return ast;

  GLMSAST *value = glms_eval(eval, args->items[0], stack);
  float v = GLMSAST_VALUE(value);

  return glms_env_new_ast_number(eval->env, cosf(v), true);
}

GLMSAST *glms_fptr_tan(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                       GLMSStack *stack) {

  if (args->length <= 0)
    return ast;

  GLMSAST *value = glms_eval(eval, args->items[0], stack);
  float v = GLMSAST_VALUE(value);

  return glms_env_new_ast_number(eval->env, tanf(v), true);
}

GLMSAST *glms_fptr_atan(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                        GLMSStack *stack) {

  if (args->length <= 0)
    return ast;

  if (args->length == 1) {
    float a = GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
    return glms_env_new_ast_number(eval->env, atanf(a), true);
  }

  float a = GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
  float b = GLMSAST_VALUE(glms_eval(eval, args->items[1], stack));
  return glms_env_new_ast_number(eval->env, atan2f(a, b), true);
}

GLMSAST *glms_fptr_fract(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                         GLMSStack *stack) {
if (args->length <= 0)
    return ast;

  GLMSAST *value = glms_eval(eval, args->items[0], stack);
  float v = GLMSAST_VALUE(value);

  return glms_env_new_ast_number(eval->env, mif_fract(v), true);
}

GLMSAST *glms_fptr_abs(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                       GLMSStack *stack) {
  if (args->length <= 0)
    return ast;

  GLMSAST *value = glms_eval(eval, args->items[0], stack);
  float v = GLMSAST_VALUE(value);

  return glms_env_new_ast_number(eval->env, fabsf(v), true);
}

GLMSAST *glms_fptr_sin(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                       GLMSStack *stack) {
  if (args->length <= 0)
    return ast;

  GLMSAST *value = glms_eval(eval, args->items[0], stack);
  float v = GLMSAST_VALUE(value);

  return glms_env_new_ast_number(eval->env, sinf(v), true);
}

GLMSAST *glms_fptr_lerp(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                        GLMSStack *stack) {

  if (args->length < 3)
    return ast;

  float from_ = GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
  float to_ = GLMSAST_VALUE(glms_eval(eval, args->items[1], stack));
  float scale_ = GLMSAST_VALUE(glms_eval(eval, args->items[2], stack));

  float v = from_ + (to_ - from_) * scale_;

  return glms_env_new_ast_number(eval->env, v, true);
}

GLMSAST *glms_fptr_clamp(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                         GLMSStack *stack) {
  if (args->length < 3)
    return ast;

  float value = GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
  float min = GLMSAST_VALUE(glms_eval(eval, args->items[1], stack));
  float max = GLMSAST_VALUE(glms_eval(eval, args->items[2], stack));

  float v = mif_clamp(value, min, max);

  return glms_env_new_ast_number(eval->env, v, true);
}

GLMSAST *glms_fptr_random(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                          GLMSStack *stack) {

  float min = 0.0f;
  float max = 1.0f;
  float seed = 32.0f;
  if (args && args->length >= 2) {
    min = GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
    max = GLMSAST_VALUE(glms_eval(eval, args->items[1], stack));
  }

  if (args && args->length >= 3) {
    seed = GLMSAST_VALUE(glms_eval(eval, args->items[2], stack));
  } else {
    seed = ((float)rand() / (float)RAND_MAX) * 321415.0f;
  }

  return glms_env_new_ast_number(eval->env, mif_random_float(min, max, seed), true);
}

GLMSAST *glms_fptr_min(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                       GLMSStack *stack) {
  if (!args || args->length <= 0)
    return ast;

  float min = INFINITY;

  for (int64_t i = 0; i < args->length; i++) {
    float v = GLMSAST_VALUE(glms_eval(eval, args->items[i], stack));

    min = fminf(min, v);
  }

  return glms_env_new_ast_number(eval->env, min, true);
}

GLMSAST *glms_fptr_max(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                       GLMSStack *stack) {

  if (!args || args->length <= 0)
    return ast;

  float max = -INFINITY;

  for (int64_t i = 0; i < args->length; i++) {
    float v = GLMSAST_VALUE(glms_eval(eval, args->items[i], stack));

    max = fmaxf(max, v);
  }

  return glms_env_new_ast_number(eval->env, max, true);
}

GLMSAST *glms_fptr_keep(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                        GLMSStack *stack) {
  if (!args || args->length <= 0) return ast;

  GLMSAST* a = glms_eval(eval, args->items[0], stack);
  glms_ast_keep(a);

  return a;
}

GLMSAST *glms_fptr_trace(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                        GLMSStack *stack) {


  glms_stack_dump(stack);

  return ast;
}

void glms_struct_vec2(GLMSEnv *env) {
  glms_env_register_struct(
      env, "vec2",
      (GLMSAST *[]){
	glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "x", true),
	glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "y", true)},
      2);
}

GLMSAST *glms_struct_vec4_swizzle(GLMSEval *eval, GLMSStack *stack,
                                  GLMSAST *ast, GLMSAST *accessor) {
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

  return glms_env_new_ast_number(eval->env, v, true);
}

GLMSAST *glms_struct_vec4_constructor(GLMSEval *eval, GLMSStack *stack,
                                      GLMSASTList *args, GLMSAST* self) {
  GLMSAST *ast = self ? self : glms_env_new_ast(eval->env, GLMS_AST_TYPE_VEC4, true);

  ast->swizzle = glms_struct_vec4_swizzle;
  ast->constructor = glms_struct_vec4_constructor;
  ast->to_string = glms_struct_vec4_to_string;

  if (!args)
    return ast;

  if (args->length >= 4) {
    float x = GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
    float y = GLMSAST_VALUE(glms_eval(eval, args->items[1], stack));
    float z = GLMSAST_VALUE(glms_eval(eval, args->items[2], stack));
    float w = GLMSAST_VALUE(glms_eval(eval, args->items[3], stack));

    ast->as.v4 = VEC4(x, y, z, w);
  } else if (args->length == 1) {
    float x = GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
    ast->as.v4 = VEC41(x);
  }

  return ast;
}

GLMSAST *glms_struct_vec3_swizzle(GLMSEval *eval, GLMSStack *stack,
                                  GLMSAST *ast, GLMSAST *accessor) {
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

  return glms_env_new_ast_number(eval->env, v, true);
}

GLMSAST *glms_struct_vec3_op_overload_mul(GLMSEval *eval, GLMSStack *stack,
                                      GLMSAST* left, GLMSAST* right) {

  if (!glms_ast_is_vector(left) && !glms_ast_is_vector(right)) return 0;

  Vector3 v = VEC31(0);
  
  if (left->type == GLMS_AST_TYPE_VEC3 && right->type == GLMS_AST_TYPE_NUMBER) {
    v = left->as.v3;
    v = vector3_scale(v, right->as.number.value);
  } else if (left->type == GLMS_AST_TYPE_NUMBER && right->type == GLMS_AST_TYPE_VEC3) {
    v = right->as.v3;
    v = vector3_scale(v, left->as.number.value);
  } else if (left->type == GLMS_AST_TYPE_VEC3 && right->type == GLMS_AST_TYPE_VEC3) {
    v = vector3_mul(left->as.v3, right->as.v3);
  }


  return glms_env_new_ast_vec3(eval->env, v, eval->arena);
}

GLMSAST *glms_struct_vec3_op_overload_div(GLMSEval *eval, GLMSStack *stack,
                                      GLMSAST* left, GLMSAST* right) {

  if (!glms_ast_is_vector(left) && !glms_ast_is_vector(right)) return 0;

  Vector3 v = VEC31(0);
  
  if (left->type == GLMS_AST_TYPE_VEC3 && right->type == GLMS_AST_TYPE_NUMBER) {
    v = left->as.v3;
    v = vector3_scale(v, 1.0f / right->as.number.value);
  } else if (left->type == GLMS_AST_TYPE_NUMBER && right->type == GLMS_AST_TYPE_VEC3) {
    v = right->as.v3;
    v = vector3_scale(v, 1.0f / left->as.number.value);
  } else if (left->type == GLMS_AST_TYPE_VEC3 && right->type == GLMS_AST_TYPE_VEC3) {
    v.x = left->as.v3.x / right->as.v3.x; 
    v.y = left->as.v3.y / right->as.v3.y; 
    v.z = left->as.v3.z / right->as.v3.z; 
  }

  return glms_env_new_ast_vec3(eval->env, v, true);
}

GLMSAST *glms_struct_vec3_constructor(GLMSEval *eval, GLMSStack *stack,
                                      GLMSASTList *args, GLMSAST* self) {
  GLMSAST *ast = self ? self : glms_env_new_ast(eval->env, GLMS_AST_TYPE_VEC3, true);

  ast->swizzle = glms_struct_vec3_swizzle;
  ast->constructor = glms_struct_vec3_constructor;
  ast->to_string = glms_struct_vec3_to_string;
  glms_ast_register_operator_overload(eval->env, ast, GLMS_TOKEN_TYPE_MUL, glms_struct_vec3_op_overload_mul);
  glms_ast_register_operator_overload(eval->env, ast, GLMS_TOKEN_TYPE_DIV, glms_struct_vec3_op_overload_div);

  if (!args)
    return ast;

  if (args->length >= 3) {
    float x = GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
    float y = GLMSAST_VALUE(glms_eval(eval, args->items[1], stack));
    float z = GLMSAST_VALUE(glms_eval(eval, args->items[2], stack));

    ast->as.v3 = VEC3(x, y, z);
  } else if (args->length == 1) {
    float x = GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
    ast->as.v3 = VEC31(x);
  }

  return ast;
}

const char *glms_struct_vec3_to_string(GLMSAST *ast) {
  Vector3 v = ast->as.v3;

  char tmp[256];
  if (ast->string_rep != 0) {
    free(ast->string_rep);
    ast->string_rep = 0;
  }
  sprintf(tmp, "VEC3(%1.6f, %1.6f, %1.6f)", v.x, v.y, v.z);
  ast->string_rep = strdup(tmp);

  return ast->string_rep;
}

const char *glms_struct_vec4_to_string(GLMSAST *ast) {
  Vector3 v = ast->as.v3;

  char tmp[256];
  if (ast->string_rep != 0) {
    free(ast->string_rep);
    ast->string_rep = 0;
  }
  sprintf(tmp, "VEC4(%1.6f, %1.6f, %1.6f, %1.6f)", v.x, v.y, v.z, v.w);
  ast->string_rep = strdup(tmp);

  return ast->string_rep;
}

void glms_struct_vec3(GLMSEnv *env) {
  glms_env_register_type(env, "vec3", glms_env_new_ast(env, GLMS_AST_TYPE_VEC3, false),
                         glms_struct_vec3_constructor, glms_struct_vec3_swizzle,
                         glms_struct_vec3_to_string, 0);
  //  glms_env_register_struct(env, "vec3", (GLMSAST*[]){
  //    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "x"),
  //    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "y"),
  //    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "z")
  //  }, 3);
}

void glms_struct_vec4(GLMSEnv *env) {
  glms_env_register_type(env, "vec4", glms_env_new_ast(env, GLMS_AST_TYPE_VEC4, false),
                         glms_struct_vec4_constructor, glms_struct_vec4_swizzle,
                         glms_struct_vec4_to_string, 0);
}



void glms_builtin_init(GLMSEnv *env) {
  srand(time(0));

  glms_env_register_any(env, "PI", glms_env_new_ast_number(env, M_PI, true));
  glms_env_register_any(env, "TAU", glms_env_new_ast_number(env, M_PI*2.0f, true));
  
  glms_env_register_function(env, "print", glms_fptr_print);
  glms_env_register_function(env, "dot", glms_fptr_dot);
  glms_env_register_function(env, "distance", glms_fptr_distance);
  glms_env_register_function(env, "cross", glms_fptr_cross);
  glms_env_register_function(env, "normalize", glms_fptr_normalize);
  glms_env_register_function(env, "unit", glms_fptr_normalize);
  glms_env_register_function(env, "length", glms_fptr_length);
  glms_env_register_function(env, "cos", glms_fptr_cos);
  glms_env_register_function(env, "sin", glms_fptr_sin);
  glms_env_register_function(env, "tan", glms_fptr_tan);
  glms_env_register_function(env, "fract", glms_fptr_fract);
  glms_env_register_function(env, "abs", glms_fptr_abs);
  glms_env_register_function(env, "atan", glms_fptr_atan);
  glms_env_register_function(env, "lerp", glms_fptr_lerp);
  glms_env_register_function(env, "mix", glms_fptr_lerp);
  glms_env_register_function(env, "clamp", glms_fptr_clamp);
  glms_env_register_function(env, "min", glms_fptr_min);
  glms_env_register_function(env, "max", glms_fptr_max);
  glms_env_register_function(env, "keep", glms_fptr_keep);
  glms_env_register_function(env, "trace", glms_fptr_trace);
  glms_env_register_function(env, "random", glms_fptr_random);

  glms_struct_image(env);
  // glms_struct_vec2(env);
  glms_struct_vec3(env);
  glms_struct_vec4(env);
}


bool glms_fptr_expect_args(GLMSEval *eval, GLMSStack *stack,
                           GLMSASTList *in_args, GLMSType *types,
                           int nr_types) {

  if (!eval || !stack) return false;
  if (types == 0 || nr_types <= 0) return true;


  if (in_args == 0 || (in_args->length <= 0 || in_args->length < nr_types)) {
    GLMS_WARNING_RETURN(eval->env->undefined, stderr, "Expected `%d` arguments but got `%ld`.\n", nr_types, in_args ? in_args->length : 0);
  }

  glms_eval_ast_list(eval, in_args, stack);

  for (int i = 0; i < nr_types; i++) {
    GLMSType type = types[i];
    GLMSAST* arg = in_args->items[i];

    const char* typename = glms_ast_get_type_name(arg);
    GLMSAST* t = glms_ast_get_type(arg);

    if (type.typename && !typename) GLMS_WARNING_RETURN(eval->env->undefined, stderr, "Unexpected non-type value.\n");

    if (type.typename && typename && strcmp(type.typename, typename) != 0) GLMS_WARNING_RETURN(eval->env->undefined, stderr, "Expected type `%s` but got `%s`.\n", type.typename, typename);


    if (t == 0) continue;

    if (!type.typename && type.ast_type != GLMS_AST_TYPE_EOF && t->type != type.ast_type) {
      GLMS_WARNING_RETURN(eval->env->undefined, stderr, "Expected type `%s` but got `%s`.\n", GLMS_AST_TYPE_STR[type.ast_type], GLMS_AST_TYPE_STR[t->type]);
    }
  }

  return true;

}
