#ifndef JSCRIPT_BUILTIN_H
#define JSCRIPT_BUILTIN_H
#include <jscript/jscript.h>


void jscript_builtin_init(JSCRIPTEnv* env);

JSCRIPTAST *jscript_fptr_print(JSCRIPTEval *eval, JSCRIPTAST *ast,
                               JSCRIPTASTList *args, JSCRIPTStack *stack);


JSCRIPTAST *jscript_fptr_dot(JSCRIPTEval *eval, JSCRIPTAST *ast,
                             JSCRIPTASTList *args, JSCRIPTStack *stack);

JSCRIPTAST *jscript_fptr_length(JSCRIPTEval *eval, JSCRIPTAST *ast,
                               JSCRIPTASTList *args, JSCRIPTStack *stack);

#endif
