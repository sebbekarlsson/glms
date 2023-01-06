#ifndef GLMS_ITERATOR_H
#define GLMS_ITERATOR_H

struct GLMS_AST_STRUCT;
struct GLMS_ENV_STRUCT;

typedef struct {
  int index;
  struct GLMS_AST_STRUCT* ast;
} GLMSIterator;

typedef int (*GLMSIteratorNext)(struct GLMS_ENV_STRUCT* env,
                                struct GLMS_AST_STRUCT* self, GLMSIterator* it,
                                struct GLMS_AST_STRUCT* out);

#endif
