#ifndef GLMS_TYPE_H
#define GLMS_TYPE_H
#include <glms/ast_type.h>
#include <glms/buffer.h>

typedef struct {
  GLMSASTType ast_type;
  char* typename;
  char* valuename;
} GLMSType;

typedef struct {
  GLMSType return_type;
  GLMSType* args;
  int args_length;
  char* description;
} GLMSFunctionSignature;

GLMS_DEFINE_BUFFER(GLMSFunctionSignature);

#define GLMS_SIGNATURE_NONE \
  (GLMSFunctionSignature) { 0 }

GLMSFunctionSignature glms_make_signature(GLMSType ret_type, GLMSType* args,
                                          int args_length);

char* glms_function_signature_to_string(GLMSFunctionSignature signa,
                                        const char* func_name);

int glms_type_destroy(GLMSType* type);
int glms_signature_destroy(GLMSFunctionSignature* signa);

#endif
