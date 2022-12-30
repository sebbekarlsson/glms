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

int glms_eval_init(GLMSEval* eval, struct GLMS_ENV_STRUCT* env);

GLMSAST glms_eval(GLMSEval *eval, GLMSAST ast, GLMSStack *stack);


GLMSAST* glms_eval_lookup(GLMSEval* eval, GLMSStack* stack, const char* key);

#endif
