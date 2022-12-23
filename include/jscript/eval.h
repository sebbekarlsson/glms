#ifndef JSCRIPT_EVAL_H
#define JSCRIPT_EVAL_H
#include <stdbool.h>
#include <jscript/ast.h>
#include <jscript/stack.h>

struct JSCRIPT_ENV_STRUCT;

typedef struct JSCRIPT_EVAL_STRUCT {
  struct JSCRIPT_ENV_STRUCT* env;
  bool initialized;
} JSCRIPTEval;

JSCRIPTAST* jscript_eval_lookup(JSCRIPTEval* eval, const char* name, JSCRIPTStack* stack);

int jscript_eval_init(JSCRIPTEval* eval, struct JSCRIPT_ENV_STRUCT* env);

JSCRIPTAST* jscript_eval(JSCRIPTEval* eval, JSCRIPTAST* ast, JSCRIPTStack* stack);

JSCRIPTAST* jscript_eval_id(JSCRIPTEval* eval, JSCRIPTAST* ast, JSCRIPTStack* stack);

JSCRIPTAST* jscript_eval_access(JSCRIPTEval* eval, JSCRIPTAST* ast, JSCRIPTStack* stack);

JSCRIPTAST* jscript_eval_binop(JSCRIPTEval* eval, JSCRIPTAST* ast, JSCRIPTStack* stack);

JSCRIPTAST* jscript_eval_unop(JSCRIPTEval* eval, JSCRIPTAST* ast, JSCRIPTStack* stack);

JSCRIPTAST* jscript_eval_compound(JSCRIPTEval* eval, JSCRIPTAST* ast, JSCRIPTStack* stack);

JSCRIPTAST* jscript_eval_function(JSCRIPTEval* eval, JSCRIPTAST* ast, JSCRIPTStack* stack);

JSCRIPTAST* jscript_eval_call(JSCRIPTEval* eval, JSCRIPTAST* ast, JSCRIPTStack* stack);

JSCRIPTAST* jscript_eval_block(JSCRIPTEval* eval, JSCRIPTAST* ast, JSCRIPTStack* stack);

#endif
