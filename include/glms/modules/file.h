#ifndef GLMS_MODULES_FILE_H
#define GLMS_MODULES_FILE_H
#include <glms/env.h>
void glms_file_type(GLMSEnv *env);

void glms_file_constructor(GLMSEval *eval, GLMSStack *stack,
			   GLMSASTBuffer *args, GLMSAST *self);
#endif
