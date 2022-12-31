#include "glms/ast.h"
#include <assert.h>
#include <glms/glms.h>
#include <glms/io.h>
#include <glms/macros.h>
#include <math.h>

#define GLMS_ASSERT(expr)                                                      \
  {                                                                            \
    if (!(expr)) {                                                             \
      GLMS_WARNING(stderr, "%s failed.\n", #expr);                             \
	exit(1);							\
    }                                                                          \
    assert(expr);							\
    printf(GLMS_CLI_GREEN "%s: OK.\n" GLMS_CLI_RESET, #expr);                  \
  }

#define GLMS_TEST_BEGIN() printf("*============= %s =============*\n", __func__)
#define GLMS_TEST_END() glms_env_clear(&env)

static GLMSAST *glms_exec_file(GLMSEnv *env, const char *path) {
  char *source = glms_get_file_contents(path);
  if (!source)
    return 0;
  glms_env_init(env, source, (GLMSConfig){});
  return glms_env_exec(env);
}

static void test_sample_var() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/var.gs");

  GLMS_ASSERT(ast != 0);
  GLMSAST *x = glms_eval_lookup(&env.eval, &env.stack, "x");

  GLMS_ASSERT(x != 0);
  GLMS_TEST_END();
}

static void test_sample_func() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/func.gs");

  GLMS_ASSERT(ast != 0);
  GLMSAST *x = glms_eval_lookup(&env.eval, &env.stack, "x");
  GLMS_ASSERT(x == 0);
  GLMSAST *f = glms_eval_lookup(&env.eval, &env.stack, "hello");
  GLMS_ASSERT(f != 0);
  GLMS_TEST_END();
}

static void test_sample_arrow_func() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/arrow_func.gs");

  GLMS_ASSERT(ast != 0);
  GLMSAST *f = glms_eval_lookup(&env.eval, &env.stack, "hello");
  GLMS_ASSERT(f != 0);
  GLMS_TEST_END();
}

static void test_sample_array() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/array.gs");

  GLMS_ASSERT(ast != 0);
  GLMSAST *x = glms_eval_lookup(&env.eval, &env.stack, "x");
  GLMS_ASSERT(x != 0);
  GLMSAST *arr = glms_eval_lookup(&env.eval, &env.stack, "arr");
  GLMS_ASSERT(arr != 0);

  GLMS_ASSERT(arr->type == GLMS_AST_TYPE_ARRAY);
  GLMS_TEST_END();
}

static void test_sample_if() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/if.gs");

  GLMS_ASSERT(ast != 0);
  GLMSAST *x = glms_eval_lookup(&env.eval, &env.stack, "x");
  GLMS_ASSERT(x != 0);
  GLMS_ASSERT(x->type == GLMS_AST_TYPE_NUMBER);
  GLMS_ASSERT(GLMSAST_VALUE(x) == 1);
  GLMS_TEST_END();
}

static void test_sample_object() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/object.gs");

  GLMS_ASSERT(ast != 0);
  GLMSAST *x = glms_eval_lookup(&env.eval, &env.stack, "x");
  GLMS_ASSERT(x != 0);
  GLMS_ASSERT(x->type == GLMS_AST_TYPE_OBJECT);

  GLMSAST *y = glms_eval_lookup(&env.eval, &env.stack, "y");
  GLMS_ASSERT(y != 0);
  GLMS_ASSERT(y->type == GLMS_AST_TYPE_NUMBER);
  GLMS_ASSERT(y->as.number.value == 33);

  GLMSAST *nested = glms_ast_access_by_key(x, "nested", &env);

  GLMS_ASSERT(nested != 0);
  GLMS_ASSERT(nested->type == GLMS_AST_TYPE_OBJECT);

  GLMSAST *other = glms_ast_access_by_key(nested, "other", &env);
  GLMS_ASSERT(other != 0);
  GLMS_ASSERT(other->type == GLMS_AST_TYPE_OBJECT);

  GLMSAST *name = glms_ast_access_by_key(other, "name", &env);
  GLMS_ASSERT(name != 0);
  GLMS_ASSERT(name->type == GLMS_AST_TYPE_STRING);
  GLMS_TEST_END();
}

static void test_sample_varfunc() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/varfunc.gs");

  GLMS_ASSERT(ast != 0);
  GLMSAST *w = glms_eval_lookup(&env.eval, &env.stack, "w");
  GLMS_ASSERT(w != 0);
  GLMS_ASSERT(w->type == GLMS_AST_TYPE_NUMBER);
  GLMS_ASSERT(GLMSAST_VALUE(w) == 44);
  GLMS_TEST_END();
}

static void test_sample_add_add() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/add_add.gs");

  GLMS_ASSERT(ast != 0);
  GLMSAST *x = glms_eval_lookup(&env.eval, &env.stack, "x");
  GLMS_ASSERT(x != 0);
  GLMS_ASSERT(x->type == GLMS_AST_TYPE_NUMBER);
  GLMS_ASSERT(GLMSAST_VALUE(x) == 1);
  GLMS_TEST_END();
}

static void test_sample_sub_sub() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/sub_sub.gs");

  GLMS_ASSERT(ast != 0);
  GLMSAST *x = glms_eval_lookup(&env.eval, &env.stack, "x");
  GLMS_ASSERT(x != 0);
  GLMS_ASSERT(x->type == GLMS_AST_TYPE_NUMBER);
  GLMS_ASSERT(GLMSAST_VALUE(x) == 2);
  GLMS_TEST_END();
}

static void test_sample_while() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/while.gs");

  GLMS_ASSERT(ast != 0);
  GLMSAST *y = glms_eval_lookup(&env.eval, &env.stack, "y");
  GLMS_ASSERT(y != 0);
  GLMS_ASSERT(y->type == GLMS_AST_TYPE_NUMBER);
  GLMS_ASSERT(GLMSAST_VALUE(y) == 500);
  GLMS_TEST_END();
}

static void test_sample_for() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/for.gs");

  GLMS_ASSERT(ast != 0);
  GLMSAST *y = glms_eval_lookup(&env.eval, &env.stack, "y");
  GLMS_ASSERT(y != 0);
  GLMS_ASSERT(y->type == GLMS_AST_TYPE_NUMBER);
  GLMS_ASSERT(GLMSAST_VALUE(y) == 16);
  GLMS_TEST_END();
}

static void test_sample_vec() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/vec.gs");
  GLMS_ASSERT(ast != 0);

  GLMSAST *v1 = glms_eval_lookup(&env.eval, &env.stack, "v1");
  GLMS_ASSERT(v1 != 0);
  GLMS_ASSERT(v1->type == GLMS_AST_TYPE_VEC3);

  GLMSAST *v2 = glms_eval_lookup(&env.eval, &env.stack, "v2");
  GLMS_ASSERT(v2 != 0);
  GLMS_ASSERT(v2->type == GLMS_AST_TYPE_VEC3);

  GLMSAST *d = glms_eval_lookup(&env.eval, &env.stack, "d");
  GLMS_ASSERT(d != 0);
  GLMS_ASSERT(d->type == GLMS_AST_TYPE_NUMBER);
  GLMS_ASSERT(d->as.number.value == 1.0f);
  GLMS_TEST_END();
}

static void test_sample_cos_sin() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/cos_sin.gs");
  GLMS_ASSERT(ast != 0);

  GLMSAST *x = glms_eval_lookup(&env.eval, &env.stack, "x");
  GLMS_ASSERT(x != 0);
  GLMS_ASSERT(x->type == GLMS_AST_TYPE_NUMBER);
  printf("%12.6f\n", x->as.number.value);
  GLMS_ASSERT(fabsf(GLMSAST_VALUE(x) - (-0.029200f)) <= 0.00001f);

  GLMSAST *y = glms_eval_lookup(&env.eval, &env.stack, "y");
  GLMS_ASSERT(y != 0);
  GLMS_ASSERT(y->type == GLMS_AST_TYPE_NUMBER);
  printf("%12.6f\n", y->as.number.value);
  GLMS_ASSERT(fabsf(GLMSAST_VALUE(y) - (0.999574f)) <= 0.00001f);

  GLMS_TEST_END();
}

static void test_sample_clamp() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/clamp.gs");
  GLMS_ASSERT(ast != 0);

  GLMSAST *value = glms_eval_lookup(&env.eval, &env.stack, "value");
  GLMS_ASSERT(value != 0);
  GLMS_ASSERT(value->type == GLMS_AST_TYPE_NUMBER);
  GLMS_ASSERT(value->as.number.value == 1.0f);

  GLMS_TEST_END();
}

static void test_sample_minmax() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/minmax.gs");
  GLMS_ASSERT(ast != 0);

  GLMSAST *x = glms_eval_lookup(&env.eval, &env.stack, "x");
  GLMS_ASSERT(x != 0);
  GLMS_ASSERT(x->type == GLMS_AST_TYPE_NUMBER);
  printf("%12.6f\n", x->as.number.value);
  GLMS_ASSERT(GLMSAST_VALUE(x) == 9.000000f);

  GLMSAST *y = glms_eval_lookup(&env.eval, &env.stack, "y");
  GLMS_ASSERT(y != 0);
  GLMS_ASSERT(y->type == GLMS_AST_TYPE_NUMBER);
  printf("%12.6f\n", y->as.number.value);
  GLMS_ASSERT(GLMSAST_VALUE(y) == 212.199997f);

  GLMS_TEST_END();
}

static void test_sample_mix() {
  GLMS_TEST_BEGIN();
  GLMSEnv env = {0};
  GLMSAST *ast = glms_exec_file(&env, "test/samples/mix.gs");
  GLMS_ASSERT(ast != 0);

  GLMSAST *v1 = glms_eval_lookup(&env.eval, &env.stack, "v1");
  GLMS_ASSERT(v1 != 0);
  GLMS_ASSERT(v1->type == GLMS_AST_TYPE_VEC3);

  GLMSAST *v2 = glms_eval_lookup(&env.eval, &env.stack, "v2");
  GLMS_ASSERT(v2 != 0);
  GLMS_ASSERT(v2->type == GLMS_AST_TYPE_VEC3);

  GLMSAST *v3 = glms_eval_lookup(&env.eval, &env.stack, "v3");
  GLMS_ASSERT(v3 != 0);

  GLMS_ASSERT(v3->type == GLMS_AST_TYPE_VEC3);

  Vector3 v3v = v3->as.v3;
  GLMS_ASSERT(v3v.x == 0.5f);
  GLMS_ASSERT(v3v.y == 0.55f);
  GLMS_ASSERT(v3v.z == 0.15f);

  GLMS_TEST_END();
}

int main(int argc, char *argv[]) {
  test_sample_var();
  test_sample_func();
  test_sample_arrow_func();
  test_sample_array();
  test_sample_if();
  //test_sample_object();
  test_sample_varfunc();
  test_sample_add_add();
  test_sample_sub_sub();
  test_sample_while();
  test_sample_for();
  test_sample_vec();
  test_sample_cos_sin();
  test_sample_clamp();
  test_sample_minmax();
  test_sample_mix();
  return 0;
}
