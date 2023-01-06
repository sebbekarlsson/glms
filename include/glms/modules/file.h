#ifndef GLMS_MODULES_FILE_H
#define GLMS_MODULES_FILE_H
#include <glms/env.h>
#include <stdint.h>
typedef struct {
  FILE *fp;
  bool open;
} GLMSFile;

typedef struct {
  GLMSFile *file;
  char *last_ptr;
  char *line;

  int64_t read_bytes;
  int64_t pos;

} GLMSFileIteratorState;

void glms_file_type(GLMSEnv *env);

void glms_file_constructor(GLMSEval *eval, GLMSStack *stack,
                           GLMSASTBuffer *args, GLMSAST *self);
#endif
