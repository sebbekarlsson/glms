#include "glms/allocator.h"
#include <glms/env.h>
#include <glms/eval.h>


char *glms_struct_vec4_to_string(GLMSAST *ast, GLMSAllocator alloc) {
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



void glms_struct_vec4_constructor(GLMSEval *eval, GLMSStack *stack,
                                      GLMSASTBuffer *args, GLMSAST* self) {
  if (!self) return;
  GLMSAST *ast = self ;//? self : glms_env_new_ast(eval->env, GLMS_AST_TYPE_VEC4, true);

  ast->type = GLMS_AST_TYPE_VEC4;
  ast->swizzle = glms_struct_vec4_swizzle;
  ast->constructor = glms_struct_vec4_constructor;
  ast->to_string = glms_struct_vec4_to_string;

  if (!args)
    return;

  if (args->length >= 4) {
    float x = (glms_eval(eval, args->items[0], stack)).as.number.value;
    float y = (glms_eval(eval, args->items[1], stack)).as.number.value;
    float z = (glms_eval(eval, args->items[2], stack)).as.number.value;
    float w = (glms_eval(eval, args->items[3], stack)).as.number.value;

    ast->as.v4 = VEC4(x, y, z, w);
  } else if (args->length == 1) {
    float x = (glms_eval(eval, args->items[0], stack)).as.number.value;
    ast->as.v4 = VEC41(x);
  }

}

void glms_struct_vec4(GLMSEnv *env) {
  glms_env_register_type(env, "vec4", glms_env_new_ast(env, GLMS_AST_TYPE_VEC4, false),
                         glms_struct_vec4_constructor, glms_struct_vec4_swizzle,
                         glms_struct_vec4_to_string, 0);
}
