#include <jscript/builtin.h>
#include <math.h>

JSCRIPTAST *jscript_fptr_print(JSCRIPTEval *eval, JSCRIPTAST *ast,
                               JSCRIPTASTList *args, JSCRIPTStack *stack) {
  if (!args)
    return ast;

  for (int64_t i = 0; i < args->length; i++) {
    JSCRIPTAST *arg = jscript_eval(eval, args->items[i], stack);
    if (!arg)
      continue;
    switch (arg->type) {
    case JSCRIPT_AST_TYPE_NUMBER: {
      printf("%1.6f\n", arg->as.number.value);
    }; break;
    case JSCRIPT_AST_TYPE_STRING: {
      const char *value = jscript_ast_get_string_value(arg);

      if (value) {
        printf("%s\n", value);
      } else {
        printf("(null)\n");
      }
    }; break;
    default: {
      printf("%p => %s\n", arg, jscript_ast_to_string(arg));
    }; break;
    }
  }

  return ast;
}

JSCRIPTAST *jscript_fptr_dot(JSCRIPTEval *eval, JSCRIPTAST *ast,
                             JSCRIPTASTList *args, JSCRIPTStack *stack) {

  if (!args) return ast;
  if (args->length < 2) return ast;

  JSCRIPTAST* a = jscript_eval(eval, args->items[0], stack);
  JSCRIPTAST* b = jscript_eval(eval, args->items[1], stack);


  float ax = jscript_ast_get_number_by_key(a, "x");
  float ay = jscript_ast_get_number_by_key(a, "y");
  float az = jscript_ast_get_number_by_key(a, "z");


  float bx = jscript_ast_get_number_by_key(b, "x");
  float by = jscript_ast_get_number_by_key(b, "y");
  float bz = jscript_ast_get_number_by_key(b, "z");


  float dot_x = ax * bx;
  float dot_y = ay * by;
  float dot_z = az * bz;
  float r = dot_x + dot_y + dot_z;

  return jscript_env_new_ast_number(eval->env, r);
}

JSCRIPTAST *jscript_fptr_length(JSCRIPTEval *eval, JSCRIPTAST *ast,
                                JSCRIPTASTList *args, JSCRIPTStack *stack) {
  if (!args) return ast;
  if (args->length <= 0) return ast;

  JSCRIPTAST* value = jscript_eval(eval, args->items[0], stack);

  float x = jscript_ast_get_number_by_key(value, "x");
  float y = jscript_ast_get_number_by_key(value, "y");
  float z = jscript_ast_get_number_by_key(value, "z");

  float r = sqrtf(powf(x, 2.0f) + powf(y, 2.0f) + powf(z, 2.0f));

  return jscript_env_new_ast_number(eval->env, r);
}

void jscript_builtin_init(JSCRIPTEnv* env) {
  jscript_env_register_function(env, "print", jscript_fptr_print);
  jscript_env_register_function(env, "dot", jscript_fptr_dot);
  jscript_env_register_function(env, "length", jscript_fptr_length);
}
