#include <jscript/jscript.h>
#include <jscript/io.h>
#include <assert.h>
#include <jscript/macros.h>

#define JSCRIPT_ASSERT(expr) {\
  printf("Test: %s\n", __func__);\
  if (!(expr)) { JSCRIPT_WARNING(stderr, "%s failed.\n", #expr);  } \
  assert(expr);\
  printf(JSCRIPT_CLI_GREEN "%s: OK.\n" JSCRIPT_CLI_RESET, #expr);\
}

static JSCRIPTAST* jscript_exec_file(JSCRIPTEnv* env, const char* path) {
  char* source = jscript_get_file_contents(path);
  if (!source) return 0;
  jscript_env_init(env, source, (JSCRIPTConfig){});
  return jscript_env_exec(env);
}

static void test_sample_var() {
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/var.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* x = jscript_eval_lookup(&env.eval, "x", &env.stack);

  JSCRIPT_ASSERT(x != 0);
}

static void test_sample_func() {
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/func.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* x = jscript_eval_lookup(&env.eval, "x", &env.stack);
  JSCRIPT_ASSERT(x == 0);
  JSCRIPTAST* f = jscript_eval_lookup(&env.eval, "hello", &env.stack);
  JSCRIPT_ASSERT(f != 0);
}


static void test_sample_arrow_func() {
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/arrow_func.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* f = jscript_eval_lookup(&env.eval, "hello", &env.stack);
  JSCRIPT_ASSERT(f != 0);
}

static void test_sample_array() {
  JSCRIPTEnv env = {0};
  JSCRIPTAST* ast = jscript_exec_file(&env, "test/samples/array.js");

  JSCRIPT_ASSERT(ast != 0);
  JSCRIPTAST* x = jscript_eval_lookup(&env.eval, "x", &env.stack); JSCRIPT_ASSERT(x != 0);
  JSCRIPTAST* arr = jscript_eval_lookup(&env.eval, "arr", &env.stack); JSCRIPT_ASSERT(arr != 0);

  JSCRIPT_ASSERT(arr->type == JSCRIPT_AST_TYPE_ARRAY);
}

int main(int argc, char* argv[]) {
  test_sample_var();
  test_sample_func();
  test_sample_arrow_func();
  test_sample_array();
  return 0;
}
