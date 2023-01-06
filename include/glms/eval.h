#ifndef GLMS_EVAL_H
#define GLMS_EVAL_H
#include <glms/ast.h>
#include <glms/stack.h>
#include <stdbool.h>

struct GLMS_ENV_STRUCT;

typedef struct GLMS_EVAL_STRUCT {
  struct GLMS_ENV_STRUCT *env;
  bool initialized;
} GLMSEval;

int glms_eval_init(GLMSEval *eval, struct GLMS_ENV_STRUCT *env);

GLMSAST glms_eval(GLMSEval *eval, GLMSAST ast, GLMSStack *stack);

GLMSAST *glms_eval_lookup(GLMSEval *eval, GLMSStack *stack, const char *key);

bool glms_eval_expect(GLMSEval *eval, GLMSStack *stack, GLMSASTType *types,
                      int nr_types, GLMSASTBuffer *args);

GLMSAST glms_eval_call_func(GLMSEval *eval, GLMSStack *stack, GLMSAST *func,
                            GLMSASTBuffer args);

GLMSAST glms_eval_assign(GLMSEval *eval, GLMSAST left, GLMSAST right,
                         GLMSStack *stack);

void glms_eval_push_args(GLMSEval *eval, GLMSStack *stack, GLMSAST *func,
                         GLMSASTBuffer args);
#endif
