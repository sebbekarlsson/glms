#include <jscript/jscript.h>
#include <jscript/io.h>
#include <assert.h>
#include <jscript/macros.h>

#define JSCRIPT_ASSERT(expr) {\
  if (!(expr)) { JSCRIPT_WARNING(stderr, "%s failed.\n", #expr);  } \
  assert(expr);\
  printf(JSCRIPT_CLI_GREEN "%s: OK.\n" JSCRIPT_CLI_RESET, #expr);\
}

#define JSCRIPT_TEST_BEGIN() printf("*============= %s =============*\n", __func__)

static JSCRIPTAST* jscript_exec_file(JSCRIPTEnv* env, const char* path) {
  char* source = jscript_get_file_contents(path);
  if (!source) return 0;
  jscript_env_init(env, source, (JSCRIPTConfig){});
  return jscript_env_exec(env);
}

static void test_sample_var() {
  JSCRIPT_TEST_BEGIN();
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/var.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* x = jscript_eval_lookup(&env.eval, "x", &env.stack);

  JSCRIPT_ASSERT(x != 0);
}

static void test_sample_func() {
  JSCRIPT_TEST_BEGIN();
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/func.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* x = jscript_eval_lookup(&env.eval, "x", &env.stack);
  JSCRIPT_ASSERT(x == 0);
  JSCRIPTAST* f = jscript_eval_lookup(&env.eval, "hello", &env.stack);
  JSCRIPT_ASSERT(f != 0);
}


static void test_sample_arrow_func() {
  JSCRIPT_TEST_BEGIN();
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/arrow_func.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* f = jscript_eval_lookup(&env.eval, "hello", &env.stack);
  JSCRIPT_ASSERT(f != 0);
}

static void test_sample_array() {
  JSCRIPT_TEST_BEGIN();
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/array.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* x = jscript_eval_lookup(&env.eval, "x", &env.stack); JSCRIPT_ASSERT(x != 0);
  JSCRIPTAST* arr = jscript_eval_lookup(&env.eval, "arr", &env.stack); JSCRIPT_ASSERT(arr != 0);

  JSCRIPT_ASSERT(arr->type == JSCRIPT_AST_TYPE_ARRAY);
}

static void test_sample_if() {
  JSCRIPT_TEST_BEGIN();
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/if.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* x = jscript_eval_lookup(&env.eval, "x", &env.stack); JSCRIPT_ASSERT(x != 0);
  JSCRIPT_ASSERT(x->type == JSCRIPT_AST_TYPE_NUMBER);
  JSCRIPT_ASSERT(JSCRIPTAST_VALUE(x) == 1);
}

static void test_sample_object() {
  JSCRIPT_TEST_BEGIN();
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/object.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* x = jscript_eval_lookup(&env.eval, "x", &env.stack); JSCRIPT_ASSERT(x != 0);
  JSCRIPT_ASSERT(x->type == JSCRIPT_AST_TYPE_OBJECT);


  JSCRIPTAST* nested = jscript_ast_access_by_key(x, "nested", &env);

  JSCRIPT_ASSERT(nested != 0);
  JSCRIPT_ASSERT(nested->type == JSCRIPT_AST_TYPE_OBJECT);

  JSCRIPTAST* other = jscript_ast_access_by_key(nested, "other", &env);
  JSCRIPT_ASSERT(other != 0);
  JSCRIPT_ASSERT(other->type == JSCRIPT_AST_TYPE_OBJECT);

  JSCRIPTAST* name = jscript_ast_access_by_key(other, "name", &env);
  JSCRIPT_ASSERT(name != 0);
  JSCRIPT_ASSERT(name->type == JSCRIPT_AST_TYPE_STRING);
}


static void test_sample_varfunc() {
  JSCRIPT_TEST_BEGIN();
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/varfunc.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* w = jscript_eval_lookup(&env.eval, "w", &env.stack); JSCRIPT_ASSERT(w != 0);
  JSCRIPT_ASSERT(w->type == JSCRIPT_AST_TYPE_NUMBER);
  JSCRIPT_ASSERT(JSCRIPTAST_VALUE(w) == 44);
}

static void test_sample_add_add() {
  JSCRIPT_TEST_BEGIN();
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/add_add.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* x = jscript_eval_lookup(&env.eval, "x", &env.stack); JSCRIPT_ASSERT(x != 0);
  JSCRIPT_ASSERT(x->type == JSCRIPT_AST_TYPE_NUMBER);
  JSCRIPT_ASSERT(JSCRIPTAST_VALUE(x) == 1);
}

static void test_sample_sub_sub() {
  JSCRIPT_TEST_BEGIN();
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/sub_sub.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* x = jscript_eval_lookup(&env.eval, "x", &env.stack); JSCRIPT_ASSERT(x != 0);
  JSCRIPT_ASSERT(x->type == JSCRIPT_AST_TYPE_NUMBER);
  JSCRIPT_ASSERT(JSCRIPTAST_VALUE(x) == 2);
}

static void test_sample_while() {
  JSCRIPT_TEST_BEGIN();
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/while.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* y = jscript_eval_lookup(&env.eval, "y", &env.stack); JSCRIPT_ASSERT(y != 0);
  JSCRIPT_ASSERT(y->type == JSCRIPT_AST_TYPE_NUMBER);
  JSCRIPT_ASSERT(JSCRIPTAST_VALUE(y) == 500);
}

static void test_sample_for() {
  JSCRIPT_TEST_BEGIN();
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/for.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* y = jscript_eval_lookup(&env.eval, "y", &env.stack); JSCRIPT_ASSERT(y != 0);
  JSCRIPT_ASSERT(y->type == JSCRIPT_AST_TYPE_NUMBER);
  JSCRIPT_ASSERT(JSCRIPTAST_VALUE(y) == 16);
}

int main(int argc, char* argv[]) {
  test_sample_var();
  test_sample_func();
  test_sample_arrow_func();
  test_sample_array();
  test_sample_if();
  test_sample_object();
  test_sample_varfunc();
  test_sample_add_add();
  test_sample_sub_sub();
  test_sample_while();
  test_sample_for();
  return 0;
}
