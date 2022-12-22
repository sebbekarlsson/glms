#include <jscript/eval.h>
#include <jscript/macros.h>
#include <jscript/env.h>

JSCRIPTAST* jscript_fptr_print(JSCRIPTEval* eval, JSCRIPTAST* ast, JSCRIPTASTList* args) {
  if (!args) return ast;

  for (int64_t i = 0; i < args->length; i++) {
    JSCRIPTAST* arg = jscript_eval(eval, args->items[i]);
    if (arg->type != JSCRIPT_AST_TYPE_NUMBER) continue;
    printf("%12.6f\n", arg->as.number.value);
  }

  return ast;
}

int jscript_eval_init(JSCRIPTEval* eval, struct JSCRIPT_ENV_STRUCT* env) {
  if (!eval || !env) return 0;
  if (eval->initialized) return 1;
  eval->initialized = true;
  eval->env = env;
  jscript_env_register_function(env, "print", jscript_fptr_print);
  return 1;
}



JSCRIPTAST* jscript_eval(JSCRIPTEval* eval, JSCRIPTAST* ast) {
  if (!eval || !ast) return 0;
  if (!eval->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "evaluator not initialized.\n");

  switch (ast->type) {
    case JSCRIPT_AST_TYPE_COMPOUND: { return jscript_eval_compound(eval, ast); }; break;
    case JSCRIPT_AST_TYPE_BINOP: { return jscript_eval_binop(eval, ast); }; break;
    case JSCRIPT_AST_TYPE_CALL: { return jscript_eval_call(eval, ast); }; break;
    default: { return ast; }; break;
  }

  return ast;
}

JSCRIPTAST* jscript_eval_call(JSCRIPTEval* eval, JSCRIPTAST* ast) {
  ast->as.call.left = jscript_eval(eval, ast->as.call.left);

  const char*  fname = jscript_string_view_get_value(&ast->as.call.left->as.id.value);

  if (!fname) return ast;

  JSCRIPTAST* func = jscript_env_lookup_function(eval->env, fname);

  if (!func || func->fptr == 0) JSCRIPT_WARNING_RETURN(ast, stderr, "No such function `%s`\n", fname);

  return func->fptr(eval, ast, ast->children);
}


JSCRIPTAST* jscript_eval_binop(JSCRIPTEval* eval, JSCRIPTAST* ast) {
  ast->as.binop.left = jscript_eval(eval, ast->as.binop.left);
  ast->as.binop.right = jscript_eval(eval, ast->as.binop.right);
  switch(ast->as.binop.op) {
    case JSCRIPT_TOKEN_TYPE_MUL:{
      return jscript_env_new_ast_number(eval->env, JSCRIPTAST_VALUE(ast->as.binop.left) * JSCRIPTAST_VALUE(ast->as.binop.right));
    };break;
    case JSCRIPT_TOKEN_TYPE_DIV:{
      return jscript_env_new_ast_number(eval->env, JSCRIPTAST_VALUE(ast->as.binop.left) / JSCRIPTAST_VALUE(ast->as.binop.right));
    };break;
    case JSCRIPT_TOKEN_TYPE_ADD:{
      return jscript_env_new_ast_number(eval->env, JSCRIPTAST_VALUE(ast->as.binop.left) + JSCRIPTAST_VALUE(ast->as.binop.right));
    };break;
    case JSCRIPT_TOKEN_TYPE_SUB:{
      return jscript_env_new_ast_number(eval->env, JSCRIPTAST_VALUE(ast->as.binop.left) - JSCRIPTAST_VALUE(ast->as.binop.right));
    };break;
    case JSCRIPT_TOKEN_TYPE_PERCENT:{
      return jscript_env_new_ast_number(eval->env, (int)JSCRIPTAST_VALUE(ast->as.binop.left) % (int)JSCRIPTAST_VALUE(ast->as.binop.right));
    };break;
    default: { return ast; }; break;
  }

  return ast;
}

JSCRIPTAST* jscript_eval_compound(JSCRIPTEval* eval, JSCRIPTAST* ast) {
  if (!ast->children || ast->children->length <= 0) return ast;
  for (int64_t i = 0; i < ast->children->length; i++) {
    JSCRIPTAST* child = ast->children->items[i];
    jscript_eval(eval, child);
  }

  return ast;
}
