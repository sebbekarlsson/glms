#ifndef GLMS_TYPE_H
#define GLMS_TYPE_H
#include <glms/ast_type.h>

typedef struct {
  GLMSASTType ast_type;
  const char* typename;
} GLMSType;
#endif
