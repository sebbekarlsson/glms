#ifndef GLMS_EVAL_H
#define GLMS_EVAL_H
#include <stdbool.h>
#include <glms/ast.h>
#include <glms/stack.h>

struct GLMS_ENV_STRUCT;

typedef struct GLMS_EVAL_STRUCT {
  struct GLMS_ENV_STRUCT* env;
  bool initialized;
} GLMSEval;

GLMSAST* glms_eval_lookup(GLMSEval* eval, const char* name, GLMSStack* stack);

int glms_eval_init(GLMSEval* eval, struct GLMS_ENV_STRUCT* env);

GLMSAST* glms_eval(GLMSEval* eval, GLMSAST* ast, GLMSStack* stack);

GLMSAST* glms_eval_id(GLMSEval* eval, GLMSAST* ast, GLMSStack* stack);

GLMSAST* glms_eval_access(GLMSEval* eval, GLMSAST* ast, GLMSStack* stack);

GLMSAST* glms_eval_binop(GLMSEval* eval, GLMSAST* ast, GLMSStack* stack);

GLMSAST* glms_eval_unop(GLMSEval* eval, GLMSAST* ast, GLMSStack* stack);

GLMSAST* glms_eval_compound(GLMSEval* eval, GLMSAST* ast, GLMSStack* stack);

GLMSAST* glms_eval_function(GLMSEval* eval, GLMSAST* ast, GLMSStack* stack);

GLMSAST* glms_eval_typedef(GLMSEval* eval, GLMSAST* ast, GLMSStack* stack);

GLMSAST* glms_eval_call(GLMSEval* eval, GLMSAST* ast, GLMSStack* stack);

GLMSAST* glms_eval_block(GLMSEval* eval, GLMSAST* ast, GLMSStack* stack);

GLMSAST* glms_eval_struct(GLMSEval* eval, GLMSAST* ast, GLMSStack* stack);

GLMSAST* glms_eval_for(GLMSEval* eval, GLMSAST* ast, GLMSStack* stack);

#endif
