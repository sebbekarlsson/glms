#ifndef GLMS_EMIT_H
#define GLMS_EMIT_H
#include <glms/string_builder.h>
#include <glms/ast.h>
#include <stdbool.h>

typedef enum { GLMS_EMIT_MODE_UNDEFINED, GLMS_EMIT_MODE_GLSL } GLMSEmitMode;

typedef struct {
  GLMSEmitMode mode;
  const char* out_path;
} GLMSEmitConfig;

typedef struct {
  GLMSEmitConfig config;
  GLMSStringBuilder builder;
  struct GLMS_ENV_STRUCT* env;
  bool initialized;
} GLMSEmit;

struct GLMS_ENV_STRUCT;

int glms_emit_init(GLMSEmit *emit, struct GLMS_ENV_STRUCT* env, GLMSEmitConfig cfg);

int glms_emit_destroy(GLMSEmit* emit);

int glms_emit(GLMSEmit* emit, GLMSAST* ast);
#endif
