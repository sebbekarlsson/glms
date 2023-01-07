#include <gimg/gimg.h>
#include <glms/ast.h>
#include <glms/builtin.h>
#include <glms/dl.h>
#include <glms/env.h>
#include <glms/macros.h>
#include <glms/math.h>
#include <glms/modules/array.h>
#include <glms/modules/fetch.h>
#include <glms/modules/file.h>
#include <glms/modules/image.h>
#include <glms/modules/iterator.h>
#include <glms/modules/json.h>
#include <glms/modules/mat4.h>
#include <glms/modules/string.h>
#include <glms/modules/vec3.h>
#include <glms/modules/vec4.h>
#include <math.h>
#include <mif/utils.h>
#include <stdlib.h>
#include <vec3/vec3.h>

#include "cglm/mat4.h"
#include "cglm/struct/quat.h"
#include "cglm/types-struct.h"
#include "glms/allocator.h"
#include "glms/ast_type.h"
#include "glms/eval.h"
#include "glms/io.h"
#include "glms/stack.h"
#include "glms/string_view.h"
#include "glms/token.h"
#include "glms/type.h"
#include "text/text.h"

static void print_ast(GLMSAST ast, GLMSAllocator alloc, GLMSEnv* env) {
  GLMSAST* t = glms_env_get_type_for(env, &ast);

  if (t && t->to_string) {
    char* buff = t->to_string(&ast, alloc, env);

    if (buff) {
      printf("%s\n", buff);
      return;
    }
  }

  if (ast.to_string) {
    char* s = ast.to_string(&ast, alloc, env);

    if (s) {
      printf("%s\n", s);
      return;
    }
  }
  switch (ast.type) {
    case GLMS_AST_TYPE_NUMBER: {
      printf("%1.6f\n", ast.as.number.value);
    }; break;
    case GLMS_AST_TYPE_STRING: {
      const char* val = glms_ast_get_string_value(&ast);
      printf("%s\n", val ? val : "(null)");
    }; break;
    case GLMS_AST_TYPE_CHAR: {
      printf("%c\n", ast.as.character.c);
    }; break;
    case GLMS_AST_TYPE_STACK_PTR: {
      GLMSAST* ptr = glms_ast_get_ptr(ast);

      if (ptr) return print_ast(*ptr, alloc, env);
      break;
      default: {
        char* v = glms_ast_to_string(ast, alloc, env);
        if (v != 0) {
          printf("%s\n", v);
        }
      }; break;
    }
  }
}

int glms_fptr_print(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                    GLMSStack* stack, GLMSAST* out) {
  if (!args) return 0;

  for (int64_t i = 0; i < args->length; i++) {
    GLMSAST arg = glms_eval(eval, args->items[i], stack);
    print_ast(arg, eval->env->string_alloc, eval->env);
  }

  return 0;
}

static void glms_dump_ast(GLMSAST ast, GLMSAllocator alloc) {
  GLMSAST* ptr = glms_ast_get_ptr(ast);

  if (ptr) return glms_dump_ast(*ptr, alloc);

  if (ast.props.initialized) {
    HashyIterator it = {0};
    while (hashy_map_iterate(&ast.props, &it)) {
      if (!it.bucket->key) continue;
      if (!it.bucket->value) continue;

      const char* key = it.bucket->key;
      GLMSAST* value = (GLMSAST*)it.bucket->value;

      printf("%s\n", key);
    }
  }
}

int glms_fptr_dump(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                   GLMSStack* stack, GLMSAST* out) {
  if (!args) return 0;

  for (int64_t i = 0; i < args->length; i++) {
    GLMSAST arg = glms_eval(eval, args->items[i], stack);
    glms_dump_ast(arg, eval->env->string_alloc);
  }

  return 0;
}

int glms_fptr_dot(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                  GLMSStack* stack, GLMSAST* out) {
  if (!args) return 0;
  if (args->length < 2) return 0;

  GLMSAST a = glms_eval(eval, args->items[0], stack);
  GLMSAST b = glms_eval(eval, args->items[1], stack);

  if (!glms_ast_is_vector(&a) || !glms_ast_is_vector(&b))
    GLMS_WARNING_RETURN(0, stderr, "Not a vector.\n");

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER,
                   .as.number.value = vector3_dot(a.as.v3, b.as.v3)};

  return 1;
}

int glms_fptr_distance(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                       GLMSStack* stack, GLMSAST* out) {
  if (!args) return 0;
  if (args->length < 2) return 0;

  GLMSAST a = glms_eval(eval, args->items[0], stack);
  GLMSAST b = glms_eval(eval, args->items[1], stack);

  if (!glms_ast_is_vector(&a) || !glms_ast_is_vector(&b))
    GLMS_WARNING_RETURN(0, stderr, "Not a vector.\n");

  float next_value = vector3_distance3d(a.as.v3, b.as.v3);
  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = next_value};
  return 1;
}

int glms_fptr_cross(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                    GLMSStack* stack, GLMSAST* out) {
  if (args->length < 2) return 0;
  GLMSAST a = glms_eval(eval, args->items[0], stack);
  GLMSAST b = glms_eval(eval, args->items[1], stack);

  if (!glms_ast_is_vector(&a) || !glms_ast_is_vector(&b))
    GLMS_WARNING_RETURN(0, stderr, "Not a vector.\n");

  *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC3,
                   .as.v3 = vector3_cross(a.as.v3, b.as.v3)};

  return 1;
}

int glms_fptr_normalize(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                        GLMSStack* stack, GLMSAST* out) {
  if (args->length <= 0) return 0;
  GLMSAST a = glms_eval(eval, args->items[0], stack);

  if (!glms_ast_is_vector(&a))
    GLMS_WARNING_RETURN(0, stderr, "Not a vector.\n");

  GLMSAST* result = glms_env_new_ast(eval->env, GLMS_AST_TYPE_VEC3, true);
  Vector3 v = vector3_unit(a.as.v3);
  *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC3, .as.v3 = v};

  return 1;
}

int glms_fptr_length(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                     GLMSStack* stack, GLMSAST* out) {
  if (!args) return 0;
  if (args->length <= 0) return 0;

  GLMSAST value = args->items[0];

  GLMSAST* ptr = glms_ast_get_ptr(value);

  // if (ptr) value = *ptr;

  if (glms_ast_is_vector(&value)) {
    *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER,
                     .as.number = vector3_mag(value.as.v3)};
    return 1;
  }

  int64_t len = 0;

  switch (value.type) {
    case GLMS_AST_TYPE_STRING: {
      const char* strvalue = glms_string_view_get_value(&value.as.string.value);
      if (!strvalue) len = 0;
      len = strlen(strvalue);
    }; break;
    default: {
      len = glms_ast_array_get_length(ast);
    }; break;
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = (float)len};

  return 1;
}

int glms_fptr_cos(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                  GLMSStack* stack, GLMSAST* out) {
  if (args->length <= 0) return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);
  float v = value.as.number.value;

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = cosf(v)};
  return 1;
}

int glms_fptr_tan(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                  GLMSStack* stack, GLMSAST* out) {
  if (args->length <= 0) return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);
  float v = value.as.number.value;

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = tanf(v)};
  return 1;
}

int glms_fptr_atan(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                   GLMSStack* stack, GLMSAST* out) {
  if (args->length <= 0) return 0;

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

int glms_fptr_fract(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                    GLMSStack* stack, GLMSAST* out) {
  if (args->length <= 0) return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);
  float v = value.as.number.value;

  *out =
      (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = mif_fract(v)};

  return 1;
}

int glms_fptr_abs(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                  GLMSStack* stack, GLMSAST* out) {
  if (args->length <= 0) return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);
  float v = value.as.number.value;

  float next_value = fabsf(v);
  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = next_value};

  return 1;
}

int glms_fptr_floor(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                  GLMSStack* stack, GLMSAST* out) {
  if (args->length <= 0) return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = floorf(glms_ast_number(value))};

  return 1;
}

int glms_fptr_ceil(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                  GLMSStack* stack, GLMSAST* out) {
  if (args->length <= 0) return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = ceilf(glms_ast_number(value))};

  return 1;
}

int glms_fptr_round(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                  GLMSStack* stack, GLMSAST* out) {
  if (args->length <= 0) return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = roundf(glms_ast_number(value))};

  return 1;
}

int glms_fptr_sin(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                  GLMSStack* stack, GLMSAST* out) {
  if (args->length <= 0) return 0;

  GLMSAST value = glms_eval(eval, args->items[0], stack);
  float v = value.as.number.value;

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = sinf(v)};

  return 1;
}

int glms_fptr_lerp(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                   GLMSStack* stack, GLMSAST* out) {
  if (args->length < 3) return 0;

  float from_ = glms_eval(eval, args->items[0], stack).as.number.value;
  float to_ = glms_eval(eval, args->items[1], stack).as.number.value;
  float scale_ = glms_eval(eval, args->items[2], stack).as.number.value;

  float v = from_ + (to_ - from_) * scale_;
  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = v};
  return 1;
}

int glms_fptr_clamp(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                    GLMSStack* stack, GLMSAST* out) {
  if (args->length < 3) return 0;

  float value = glms_eval(eval, args->items[0], stack).as.number.value;
  float min = glms_eval(eval, args->items[1], stack).as.number.value;
  float max = glms_eval(eval, args->items[2], stack).as.number.value;

  float v = mif_clamp(value, min, max);

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = v};

  return 1;
}

int glms_fptr_random(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                     GLMSStack* stack, GLMSAST* out) {
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

int glms_fptr_min(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                  GLMSStack* stack, GLMSAST* out) {
  if (!args || args->length <= 0) return 0;

  float min = INFINITY;

  for (int64_t i = 0; i < args->length; i++) {
    float v = glms_eval(eval, args->items[i], stack).as.number.value;

    min = fminf(min, v);
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = min};
  return 1;
}

int glms_fptr_max(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                  GLMSStack* stack, GLMSAST* out) {
  if (!args || args->length <= 0) return 0;

  float max = -INFINITY;

  for (int64_t i = 0; i < args->length; i++) {
    float v = glms_eval(eval, args->items[i], stack).as.number.value;

    max = fmaxf(max, v);
  }

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = max};
  return 1;
}

int glms_fptr_pow(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                  GLMSStack* stack, GLMSAST* out) {
  glms_eval_expect(eval, stack,
                   (GLMSASTType[]){GLMS_AST_TYPE_NUMBER, GLMS_AST_TYPE_NUMBER},
                   2, args);

  float x = glms_ast_number(args->items[0]);
  float y = glms_ast_number(args->items[1]);

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = powf(x, y)};

  return 1;
}

int glms_fptr_log(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                  GLMSStack* stack, GLMSAST* out) {
  glms_eval_expect(eval, stack, (GLMSASTType[]){GLMS_AST_TYPE_NUMBER}, 1, args);

  float x = glms_ast_number(args->items[0]);
  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = logf(x)};

  return 1;
}

int glms_fptr_log10(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                    GLMSStack* stack, GLMSAST* out) {
  glms_eval_expect(eval, stack, (GLMSASTType[]){GLMS_AST_TYPE_NUMBER}, 1, args);

  float x = glms_ast_number(args->items[0]);
  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = log10f(x)};

  return 1;
}

int glms_fptr_perspective(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                          GLMSStack* stack, GLMSAST* out) {
  float fov = glms_ast_number(args->items[0]);
  float aspect = glms_ast_number(args->items[1]);
  float near = glms_ast_number(args->items[2]);
  float far = glms_ast_number(args->items[3]);

  GLMSAST result = (GLMSAST){.type = GLMS_AST_TYPE_MAT4,
                             .as.m4 = glms_perspective(fov, aspect, near, far)};

  mat4s m = result.as.m4;

  *out = result;

  return 1;
}

int glms_fptr_ortho(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                    GLMSStack* stack, GLMSAST* out) {
  float left = glms_ast_number(args->items[0]);
  float right = glms_ast_number(args->items[1]);
  float bottom = glms_ast_number(args->items[2]);
  float top = glms_ast_number(args->items[3]);
  float near = glms_ast_number(args->items[4]);
  float far = glms_ast_number(args->items[5]);

  GLMSAST result =
      (GLMSAST){.type = GLMS_AST_TYPE_MAT4,
                .as.m4 = glms_ortho(left, right, bottom, top, near, far)};

  *out = result;

  return 1;
}

int glms_fptr_identity(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                       GLMSStack* stack, GLMSAST* out) {
  GLMSAST result =
      (GLMSAST){.type = GLMS_AST_TYPE_MAT4, .as.m4 = glms_mat4_identity()};
  *out = result;

  return 1;
}

int glms_fptr_transpose(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                        GLMSStack* stack, GLMSAST* out) {
  GLMSAST result =
      (GLMSAST){.type = GLMS_AST_TYPE_MAT4,
                .as.m4 = glms_mat4_transpose(args->items[0].as.m4)};
  *out = result;

  glms_env_apply_type(eval->env, eval, stack, out);
  return 1;
}

int glms_fptr_inverse(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                      GLMSStack* stack, GLMSAST* out) {
  GLMSAST result = (GLMSAST){.type = GLMS_AST_TYPE_MAT4,
                             .as.m4 = glms_mat4_inv(args->items[0].as.m4)};
  *out = result;

  return 1;
}

int glms_fptr_radians(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                      GLMSStack* stack, GLMSAST* out) {
  float v = glms_ast_number(args->items[0]);

  GLMSAST result =
      (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = glm_rad(v)};
  *out = result;

  return 1;
}

int glms_fptr_smoothstep(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                         GLMSStack* stack, GLMSAST* out) {
  GLMSAST arg0 = args->items[0];
  GLMSAST arg1 = args->items[1];
  GLMSAST arg2 = args->items[2];

  if (arg0.type == GLMS_AST_TYPE_NUMBER && arg1.type == GLMS_AST_TYPE_NUMBER &&
      arg2.type == GLMS_AST_TYPE_NUMBER) {
    float edge0 = glms_ast_number(arg0);
    float edge1 = glms_ast_number(arg1);
    float f = glms_ast_number(arg2);
    *out =
        (GLMSAST){.type = GLMS_AST_TYPE_NUMBER,
                  .as.number.value = glms_smoothstep_factor(edge0, edge1, f)};
    return 1;
  } else if (arg0.type == GLMS_AST_TYPE_VEC3 &&
             arg1.type == GLMS_AST_TYPE_VEC3 &&
             arg2.type == GLMS_AST_TYPE_NUMBER) {
    Vector3 edge0 = arg0.as.v3;
    Vector3 edge1 = arg1.as.v3;
    float f = glms_ast_number(arg2);
    *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC3,
                     .as.v3 = glms_smoothstep_vec3_factor(edge0, edge1, f)};
    return 1;
  } else if (arg0.type == GLMS_AST_TYPE_VEC3 &&
             arg1.type == GLMS_AST_TYPE_VEC3 &&
             arg2.type == GLMS_AST_TYPE_VEC3) {
    Vector3 edge0 = arg0.as.v3;
    Vector3 edge1 = arg1.as.v3;
    Vector3 f = arg2.as.v3;
    *out = (GLMSAST){.type = GLMS_AST_TYPE_VEC3,
                     .as.v3 = glms_smoothstep_vec3_vec3(edge0, edge1, f)};
    return 1;
  }

  return 0;
}

int glms_fptr_trace(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                    GLMSStack* stack, GLMSAST* out) {
  glms_stack_dump(stack, eval->env);

  return 0;
}

int glms_fptr_quat_for(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                       GLMSStack* stack, GLMSAST* out) {
  Vector3 dir = args->items[0].as.v3;
  Vector3 up = args->items[1].as.v3;
  versors q =
      glms_quat_for((vec3s){dir.x, dir.y, dir.z}, (vec3s){up.x, up.y, up.z});

  GLMSAST result =
      (GLMSAST){.type = GLMS_AST_TYPE_VEC4,
                .as.v4 = VEC4(q.raw[0], q.raw[1], q.raw[2], q.raw[3])};
  *out = result;

  glms_env_apply_type(eval->env, eval, stack, out);
  return 1;
}

int glms_fptr_cantor(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                     GLMSStack* stack, GLMSAST* out) {
  if (args->length <= 0) return 0;

  float x = glms_ast_number(args->items[0]);
  float y = glms_ast_number(args->items[1]);

  int c = mif_cantor((int)x, (int)y);

  *out = (GLMSAST){.type = GLMS_AST_TYPE_NUMBER, .as.number.value = (float)c};

  return 1;
}

int glms_fptr_decant(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                     GLMSStack* stack, GLMSAST* out) {
  if (args->length <= 0) return 0;

  float x = glms_ast_number(args->items[0]);

  GLMSAST* new_array = glms_env_new_ast(eval->env, GLMS_AST_TYPE_ARRAY, true);

  int a = 0;
  int b = 0;

  mif_decant((int)x, &a, &b);

  glms_ast_push(new_array, glms_env_new_ast_number(eval->env, a, true));
  glms_ast_push(new_array, glms_env_new_ast_number(eval->env, b, true));

  *out =
      (GLMSAST){.type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_array};

  return 1;
}

int glms_fptr_exit(GLMSEval* eval, GLMSAST* ast, GLMSASTBuffer* args,
                   GLMSStack* stack, GLMSAST* out) {
  int x = args != 0 && args->length > 0 ? glms_ast_number(args->items[0]) : 0;

  exit(x);

  return 1;
}

void glms_builtin_init(GLMSEnv* env) {
  if (env->has_builtins) return;
  env->has_builtins = true;
  srand(time(0));

  glms_env_register_any(env, "PI", glms_env_new_ast_number(env, M_PI, true));
  glms_env_register_any(env, "TAU",
                        glms_env_new_ast_number(env, M_PI * 2.0f, true));
  glms_env_register_function(env, "print", glms_fptr_print);
  glms_env_register_function(env, "dump", glms_fptr_dump);
  glms_env_register_function(env, "trace", glms_fptr_trace);
  glms_env_register_function(env, "exit", glms_fptr_exit);
  glms_env_register_function(env, "quit", glms_fptr_exit);
  glms_env_register_function(env, "quatFor", glms_fptr_quat_for);
  glms_env_register_function(env, "cantor", glms_fptr_cantor);
  glms_env_register_function_signature(
      env, 0, "cantor",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER},
                               (GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 2});

  glms_env_register_function(env, "decant", glms_fptr_decant);
  glms_env_register_function_signature(
      env, 0, "decant",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_ARRAY},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});

  glms_env_register_function_signature(
      env, 0, "quatFor",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_VEC4},
          .args =
              (GLMSType[]){(GLMSType){GLMS_AST_TYPE_VEC4, .valuename = "dir"},
                           (GLMSType){GLMS_AST_TYPE_VEC4, .valuename = "up"}},
          .args_length = 2});

  glms_env_register_function(env, "smoothstep", glms_fptr_smoothstep);
  glms_env_register_function_signature(
      env, 0, "smoothstep",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args =
              (GLMSType[]){
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "edge0"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "edge1"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "value"}},
          .args_length = 3});
  glms_env_register_function_signature(
      env, 0, "smoothstep",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_VEC3},
          .args =
              (GLMSType[]){
                  (GLMSType){GLMS_AST_TYPE_VEC3, .valuename = "edge0"},
                  (GLMSType){GLMS_AST_TYPE_VEC3, .valuename = "edge1"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "value"}},
          .args_length = 3});
  glms_env_register_function_signature(
      env, 0, "smoothstep",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_VEC3},
          .args =
              (GLMSType[]){
                  (GLMSType){GLMS_AST_TYPE_VEC3, .valuename = "edge0"},
                  (GLMSType){GLMS_AST_TYPE_VEC3, .valuename = "edge1"},
                  (GLMSType){GLMS_AST_TYPE_VEC3, .valuename = "value"}},
          .args_length = 3});

  glms_env_register_function(env, "radians", glms_fptr_radians);
  glms_env_register_function_signature(
      env, 0, "radians",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});

  glms_env_register_function(env, "identity", glms_fptr_identity);
  glms_env_register_function_signature(
      env, 0, "identity",
      (GLMSFunctionSignature){.return_type = (GLMSType){GLMS_AST_TYPE_MAT4},
                              .args_length = 0});

  glms_env_register_function(env, "transpose", glms_fptr_identity);
  glms_env_register_function_signature(
      env, 0, "transpose",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_MAT4},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_MAT4}},
          .args_length = 1});

  glms_env_register_function(env, "inverse", glms_fptr_identity);
  glms_env_register_function_signature(
      env, 0, "inverse",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_MAT4},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_MAT4}},
          .args_length = 1});

  glms_env_register_function(env, "ortho", glms_fptr_ortho);
  glms_env_register_function_signature(
      env, 0, "ortho",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_MAT4},
          .args =
              (GLMSType[]){
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "left"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "right"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "bottom"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "top"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "near"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "far"}},
          .args_length = 6});

  glms_env_register_function(env, "perspective", glms_fptr_perspective);
  glms_env_register_function_signature(
      env, 0, "perspective",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_MAT4},
          .args =
              (GLMSType[]){
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "fov"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "aspect"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "near"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "far"}},
          .args_length = 4});

  glms_env_register_function(env, "dot", glms_fptr_dot);
  glms_env_register_function_signature(
      env, 0, "dot",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_VEC3},
                               (GLMSType){GLMS_AST_TYPE_VEC3}},
          .args_length = 2});

  glms_env_register_function(env, "distance", glms_fptr_distance);
  glms_env_register_function_signature(
      env, 0, "distance",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_VEC3},
                               (GLMSType){GLMS_AST_TYPE_VEC3}},
          .args_length = 2});

  glms_env_register_function(env, "cross", glms_fptr_cross);
  glms_env_register_function_signature(
      env, 0, "cross",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_VEC3},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_VEC3},
                               (GLMSType){GLMS_AST_TYPE_VEC3}},
          .args_length = 2});

  glms_env_register_function(env, "normalize", glms_fptr_normalize);
  glms_env_register_function_signature(
      env, 0, "normalize",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_VEC3},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_VEC3}},
          .args_length = 1});

  glms_env_register_function(env, "unit", glms_fptr_normalize);
  glms_env_register_function_signature(
      env, 0, "unit",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_VEC3},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_VEC3}},
          .args_length = 1});

  glms_env_register_function(env, "length", glms_fptr_length);
  glms_env_register_function_signature(
      env, 0, "length",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_VEC3}},
          .args_length = 1});
  glms_env_register_function_signature(
      env, 0, "length",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_STRING}},
          .args_length = 1});
  glms_env_register_function_signature(
      env, 0, "length",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_ARRAY}},
          .args_length = 1});

  glms_env_register_function(env, "cos", glms_fptr_cos);
  glms_env_register_function_signature(
      env, 0, "cos",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});

  glms_env_register_function(env, "sin", glms_fptr_sin);
  glms_env_register_function_signature(
      env, 0, "sin",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});

  glms_env_register_function(env, "tan", glms_fptr_tan);
  glms_env_register_function_signature(
      env, 0, "tan",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});

  glms_env_register_function(env, "fract", glms_fptr_fract);
  glms_env_register_function_signature(
      env, 0, "fract",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});

  glms_env_register_function(env, "abs", glms_fptr_abs);
  glms_env_register_function_signature(
      env, 0, "abs",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});

  glms_env_register_function(env, "floor", glms_fptr_floor);
  glms_env_register_function_signature(
      env, 0, "floor",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});

  glms_env_register_function(env, "ceil", glms_fptr_ceil);
  glms_env_register_function_signature(
      env, 0, "ceil",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});

  glms_env_register_function(env, "round", glms_fptr_round);
  glms_env_register_function_signature(
      env, 0, "round",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});

  glms_env_register_function(env, "atan", glms_fptr_atan);
  glms_env_register_function_signature(
      env, 0, "atan",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});
  glms_env_register_function_signature(
      env, 0, "atan",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER},
                               (GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 2});

  glms_env_register_function(env, "lerp", glms_fptr_lerp);
  glms_env_register_function_signature(
      env, 0, "lerp",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args =
              (GLMSType[]){
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "from"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "to"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "scale"}},
          .args_length = 3});
  glms_env_register_function_signature(
      env, 0, "lerp",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_VEC3},
          .args =
              (GLMSType[]){
                  (GLMSType){GLMS_AST_TYPE_VEC3, .valuename = "from"},
                  (GLMSType){GLMS_AST_TYPE_VEC3, .valuename = "to"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "scale"}},
          .args_length = 3});

  glms_env_register_function(env, "mix", glms_fptr_lerp);
  glms_env_register_function_signature(
      env, 0, "mix",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args =
              (GLMSType[]){
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "from"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "to"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "scale"}},
          .args_length = 3});
  glms_env_register_function_signature(
      env, 0, "mix",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_VEC3},
          .args =
              (GLMSType[]){
                  (GLMSType){GLMS_AST_TYPE_VEC3, .valuename = "from"},
                  (GLMSType){GLMS_AST_TYPE_VEC3, .valuename = "to"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "scale"}},
          .args_length = 3});

  glms_env_register_function(env, "clamp", glms_fptr_clamp);
  glms_env_register_function_signature(
      env, 0, "clamp",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args =
              (GLMSType[]){
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "value"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "min"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "max"}},
          .args_length = 3});

  glms_env_register_function(env, "min", glms_fptr_min);
  glms_env_register_function_signature(
      env, 0, "min",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER},
                               (GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 2});

  glms_env_register_function(env, "max", glms_fptr_max);
  glms_env_register_function_signature(
      env, 0, "max",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER},
                               (GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 2});

  glms_env_register_function(env, "pow", glms_fptr_pow);
  glms_env_register_function_signature(
      env, 0, "pow",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER},
                               (GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 2});

  glms_env_register_function(env, "log", glms_fptr_log);
  glms_env_register_function_signature(
      env, 0, "log",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});

  glms_env_register_function(env, "log10", glms_fptr_log10);
  glms_env_register_function_signature(
      env, 0, "log10",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args = (GLMSType[]){(GLMSType){GLMS_AST_TYPE_NUMBER}},
          .args_length = 1});

  glms_env_register_function(env, "random", glms_fptr_random);
  glms_env_register_function_signature(
      env, 0, "random",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args_length = 0,
          .description = "Returns a random value between 0 and 1."});
  glms_env_register_function_signature(
      env, 0, "random",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args =
              (GLMSType[]){
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "min"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "max"}},
          .args_length = 2});
  glms_env_register_function_signature(
      env, 0, "random",
      (GLMSFunctionSignature){
          .return_type = (GLMSType){GLMS_AST_TYPE_NUMBER},
          .args =
              (GLMSType[]){
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "min"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "max"},
                  (GLMSType){GLMS_AST_TYPE_NUMBER, .valuename = "seed"}},
          .args_length = 3});

  glms_string_type(env);
  glms_array_type(env);
  glms_iterator_type(env);
  glms_struct_vec3(env);
  glms_struct_vec4(env);
  glms_mat4_type(env);
  glms_struct_image(env);
  glms_file_type(env);
  glms_fetch(env);
  glms_json(env);
}
