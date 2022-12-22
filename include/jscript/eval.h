#ifndef JSCRIPT_EVAL_H
#define JSCRIPT_EVAL_H
#include <stdbool.h>
#include <jscript/ast.h>

struct JSCRIPT_ENV_STRUCT;

typedef struct JSCRIPT_EVAL_STRUCT {
  struct JSCRIPT_ENV_STRUCT* env;
  bool initialized;
} JSCRIPTEval;


int jscript_eval_init(JSCRIPTEval* eval, struct JSCRIPT_ENV_STRUCT* env);

JSCRIPTAST* jscript_eval(JSCRIPTEval* eval, JSCRIPTAST* ast);

JSCRIPTAST* jscript_eval_binop(JSCRIPTEval* eval, JSCRIPTAST* ast);

JSCRIPTAST* jscript_eval_compound(JSCRIPTEval* eval, JSCRIPTAST* ast);

JSCRIPTAST* jscript_eval_call(JSCRIPTEval* eval, JSCRIPTAST* ast);

#endif
