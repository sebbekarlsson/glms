#ifndef GLMS_FPTR_H
#define GLMS_FPTR_H

struct GLMS_STACK_STRUCT;
struct GLMS_AST_STRUCT;
struct GLMS_EVAL_STRUCT;
struct GLMS_GLMSAST_LIST_STRUCT;
struct GLMS_BUFFER_GLMSAST;

typedef int (*GLMSFPTR)(struct GLMS_EVAL_STRUCT* eval,
                        struct GLMS_AST_STRUCT* ast,
                        struct GLMS_BUFFER_GLMSAST* args,
                        struct GLMS_STACK_STRUCT* stack,
                        struct GLMS_AST_STRUCT* out);
#endif
