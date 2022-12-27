#include <glms/builtin.h>
#include <math.h>

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
      printf("%p => %s\n", arg, glms_ast_to_string(arg));
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

  float ax = glms_ast_get_number_by_key(a, "x");
  float ay = glms_ast_get_number_by_key(a, "y");
  float az = glms_ast_get_number_by_key(a, "z");

  float bx = glms_ast_get_number_by_key(b, "x");
  float by = glms_ast_get_number_by_key(b, "y");
  float bz = glms_ast_get_number_by_key(b, "z");

  float dot_x = ax * bx;
  float dot_y = ay * by;
  float dot_z = az * bz;
  float r = dot_x + dot_y + dot_z;

  return glms_env_new_ast_number(eval->env, r);
}

GLMSAST *glms_fptr_length(GLMSEval *eval, GLMSAST *ast, GLMSASTList *args,
                          GLMSStack *stack) {
  if (!args)
    return ast;
  if (args->length <= 0)
    return ast;

  GLMSAST *value = glms_eval(eval, args->items[0], stack);

  float x = glms_ast_get_number_by_key(value, "x");
  float y = glms_ast_get_number_by_key(value, "y");
  float z = glms_ast_get_number_by_key(value, "z");

  float r = sqrtf(powf(x, 2.0f) + powf(y, 2.0f) + powf(z, 2.0f));

  return glms_env_new_ast_number(eval->env, r);
}

GLMSAST *glms_fptr_cos(GLMSEval *eval, GLMSAST *ast,
                       GLMSASTList *args, GLMSStack *stack) {

  if (args->length <= 0)
    return ast;

  GLMSAST *value = glms_eval(eval, args->items[0], stack);
  float v = GLMSAST_VALUE(value);

  return glms_env_new_ast_number(eval->env, cosf(v));
}

GLMSAST *glms_fptr_sin(GLMSEval *eval, GLMSAST *ast,
                       GLMSASTList *args, GLMSStack *stack) {
  if (args->length <= 0)
    return ast;

  GLMSAST *value = glms_eval(eval, args->items[0], stack);
  float v = GLMSAST_VALUE(value);

  return glms_env_new_ast_number(eval->env, sinf(v));
}

GLMSAST *glms_fptr_lerp(GLMSEval *eval, GLMSAST *ast,
                        GLMSASTList *args, GLMSStack *stack) {

  if (args->length < 3)
    return ast;

  float from_ = GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
  float to_ = GLMSAST_VALUE(glms_eval(eval, args->items[1], stack));
  float scale_ = GLMSAST_VALUE(glms_eval(eval, args->items[2], stack));

  float v = from_ + (to_ - from_) * scale_;

  return glms_env_new_ast_number(eval->env, v);
}

void glms_struct_vec2(GLMSEnv *env) {
  glms_env_register_struct(env, "vec2", (GLMSAST*[]){
    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "x"),
    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "y")
  }, 2);
}

void glms_struct_vec3(GLMSEnv *env) {
  glms_env_register_struct(env, "vec3", (GLMSAST*[]){
    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "x"),
    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "y"),
    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "z")
  }, 3);
}

void glms_struct_vec4(GLMSEnv *env) {
  glms_env_register_struct(env, "vec4", (GLMSAST*[]){
    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "x"),
    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "y"),
    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "z"),
    glms_env_new_ast_field(env, GLMS_TOKEN_TYPE_SPECIAL_NUMBER, "w")
  }, 4);
}

void glms_builtin_init(GLMSEnv *env) {
  glms_env_register_function(env, "print", glms_fptr_print);
  glms_env_register_function(env, "dot", glms_fptr_dot);
  glms_env_register_function(env, "length", glms_fptr_length);
  glms_env_register_function(env, "cos", glms_fptr_cos);
  glms_env_register_function(env, "sin", glms_fptr_sin);
  glms_env_register_function(env, "lerp", glms_fptr_lerp);
  glms_struct_vec2(env);
  glms_struct_vec3(env);
  glms_struct_vec4(env);
}
