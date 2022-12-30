#include "glms/allocator.h"
#include "glms/ast_type.h"
#include "glms/eval.h"
#include "glms/stack.h"
#include "glms/string_view.h"
#include "glms/token.h"
#include "text/text.h"
#include <gimg/gimg.h>
#include <glms/ast.h>
#include <glms/builtin.h>
#include <glms/env.h>
#include <glms/macros.h>
#include <glms/modules/image.h>
#include <glms/modules/vec3.h>
#include <glms/modules/vec4.h>
#include <math.h>
#include <mif/utils.h>
#include <stdlib.h>
#include <vec3/vec3.h>

static void print_ast(GLMSAST ast, GLMSAllocator alloc) {

  switch (ast.type) {
  case GLMS_AST_TYPE_NUMBER: {
    printf("%1.6f\n", ast.as.number.value);
  }; break;
  case GLMS_AST_TYPE_STRING: {
    const char *val = glms_string_view_get_value(&ast.as.string.value);
    printf("%s\n", val ? val : "");
  }; break;
  case GLMS_AST_TYPE_STACK_PTR: {

    if (ast.as.stackptr.ptr) {
      return print_ast(*ast.as.stackptr.ptr, alloc);
    };
    break;
  default: {
    char* v = glms_ast_to_string(ast, alloc);
    if (v != 0) {
      printf("%s\n", v);
    }
  }; break;
  }
  }
}

int glms_fptr_print(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                    GLMSStack *stack, GLMSAST *out) {
  if (!args)
    return 0;

  for (int64_t i = 0; i < args->length; i++) {
    GLMSAST arg = glms_eval(eval, args->items[i], stack);
    print_ast(arg, eval->env->string_alloc);
  }

  return 0;
}

int glms_fptr_dot(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out) {

  if (!args)
    return 0;
  if (args->length < 2)
    return 0;

  GLMSAST a = glms_eval(eval, args->items[0], stack);
  GLMSAST b = glms_eval(eval, args->items[1], stack);

  if (!glms_ast_is_vector(&a) || !glms_ast_is_vector(&b))
    GLMS_WARNING_RETURN(0, stderr, "Not a vector.\n");

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER,
                   .as.number.value = vector3_dot(a.as.v3, b.as.v3)};

  return 1;
}

int glms_fptr_distance(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                       GLMSStack *stack, GLMSAST *out) {
  if (!args)
    return 0;
  if (args->length < 2)
    return 0;

  GLMSAST a = glms_eval(eval, args->items[0], stack);
  GLMSAST b = glms_eval(eval, args->items[1], stack);

  if (!glms_ast_is_vector(&a) || !glms_ast_is_vector(&b))
    GLMS_WARNING_RETURN(0, stderr, "Not a vector.\n");

  float next_value = vector3_distance3d(a.as.v3, b.as.v3);
  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = next_value};
  return 1;
}

int glms_fptr_cross(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                    GLMSStack *stack, GLMSAST *out) {

  if (args->length < 2)
    return 0;
  GLMSAST a = glms_eval(eval, args->items[0], stack);
  GLMSAST b = glms_eval(eval, args->items[1], stack);

  if (!glms_ast_is_vector(&a) || !glms_ast_is_vector(&b))
    GLMS_WARNING_RETURN(0, stderr, "Not a vector.\n");

  *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC3,
                   .as.v3 = vector3_cross(a.as.v3, b.as.v3)};

  return 1;
}

int glms_fptr_normalize(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                        GLMSStack *stack, GLMSAST *out) {

  if (args->length <= 0)
    return 0;
  GLMSAST a = glms_eval(eval, args->items[0], stack);

  if (!glms_ast_is_vector(&a))
    GLMS_WARNING_RETURN(0, stderr, "Not a vector.\n");

  GLMSAST *result = glms_env_new_ast(eval->env, GLMS_AST_TYPE_VEC3, true);
  Vector3 v = vector3_unit(a.as.v3);
  *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC3, .as.v3 = v};

  return 1;
}

int glms_fptr_length(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                     GLMSStack *stack, GLMSAST *out) {
  if (!args)
    return 0;
  if (args->length <= 0)
    return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);

  if (glms_ast_is_vector(&value)) {

    *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER,
                     .as.number = vector3_mag(value.as.v3)};
    return 1;
  }

  int64_t len = 0;

  switch (value.type) {
  case GLMS_AST_TYPE_STRING: {
    const char *strvalue = glms_string_view_get_value(&value.as.string.value);
    if (!strvalue)
      len = 0;
    len = strlen(strvalue);
  }; break;
  default: {
    len = glms_ast_array_get_length(ast);
  }; break;
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = (float)len};

  return 1;
}

int glms_fptr_cos(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out) {

  if (args->length <= 0)
    return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);
  float v = value.as.number.value;

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = cosf(v)};
  return 1;
}

int glms_fptr_tan(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out) {

  if (args->length <= 0)
    return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);
  float v = value.as.number.value;

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = tanf(v)};
  return 1;
}

int glms_fptr_atan(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                   GLMSStack *stack, GLMSAST *out) {

  if (args->length <= 0)
    return 0;

  if (args->length == 1) {
    float a = args->items[0].as.number.value;
    *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = atanf(a)};
    return 1;
  }

  float a = args->items[0].as.number.value;
  float b = args->items[1].as.number.value;
  *out =
      (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = atan2f(a, b)};
  return 1;
}

int glms_fptr_fract(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                    GLMSStack *stack, GLMSAST *out) {
  if (args->length <= 0)
    return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);
  float v = value.as.number.value;

  *out =
      (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = mif_fract(v)};

  return 1;
}

int glms_fptr_abs(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out) {
  if (args->length <= 0)
    return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);
  float v = value.as.number.value;

  float next_value = fabsf(v);
  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = next_value};

  return 1;
}

int glms_fptr_sin(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out) {
  if (args->length <= 0)
    return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);
  float v = value.as.number.value;

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = sinf(v)};

  return 1;
}

int glms_fptr_lerp(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                   GLMSStack *stack, GLMSAST *out) {

  if (args->length < 3)
    return 0;

  float from_ = glms_eval(eval, args->items[0], stack).as.number.value;
  float to_ = glms_eval(eval, args->items[1], stack).as.number.value;
  float scale_ = glms_eval(eval, args->items[2], stack).as.number.value;

  float v = from_ + (to_ - from_) * scale_;
  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = v};
  return 1;
}

int glms_fptr_clamp(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                    GLMSStack *stack, GLMSAST *out) {
  if (args->length < 3)
    return 0;

  float value = glms_eval(eval, args->items[0], stack).as.number.value;
  float min = glms_eval(eval, args->items[1], stack).as.number.value;
  float max = glms_eval(eval, args->items[2], stack).as.number.value;

  float v = mif_clamp(value, min, max);

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = v};

  return 1;
}

int glms_fptr_random(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                     GLMSStack *stack, GLMSAST *out) {

  float min = 0.0f;
  float max = 1.0f;
  float seed = 32.0f;
  if (args && args->length >= 2) {
    min = glms_eval(eval, args->items[0], stack).as.number.value;
    max = glms_eval(eval, args->items[1], stack).as.number.value;
  }

  if (args && args->length >= 3) {
    seed = glms_eval(eval, args->items[2], stack).as.number.value;
  } else {
    seed = ((float)rand() / (float)RAND_MAX) * 321415.0f;
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER,
                   .as.number.value = mif_random_float(min, max, seed)};
  return 1;
}

int glms_fptr_min(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out) {
  if (!args || args->length <= 0)
    return 0;

  float min = INFINITY;

  for (int64_t i = 0; i < args->length; i++) {
    float v = glms_eval(eval, args->items[i], stack).as.number.value;

    min = fminf(min, v);
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = min};
  return 1;
}

int glms_fptr_max(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                  GLMSStack *stack, GLMSAST *out) {

  if (!args || args->length <= 0)
    return 0;

  float max = -INFINITY;

  for (int64_t i = 0; i < args->length; i++) {
    float v = glms_eval(eval, args->items[i], stack).as.number.value;

    max = fmaxf(max, v);
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = max};
  return 1;
}



void glms_builtin_init(GLMSEnv *env) {
  srand(time(0));

  glms_env_register_any(env, "PI", glms_env_new_ast_number(env, M_PI, true));
  glms_env_register_any(env, "TAU",
                        glms_env_new_ast_number(env, M_PI * 2.0f, true));
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
  glms_env_register_function(env, "random", glms_fptr_random);

  glms_struct_vec3(env);
  glms_struct_vec4(env);
  glms_struct_image(env);
}
