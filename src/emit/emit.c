#include "glms/env.h"
#include "glms/macros.h"
#include "glms/string_builder.h"
#include <glms/emit/emit.h>
#include <glms/emit/glsl/emit_glsl.h>
#include <stdio.h>
#include <stdlib.h>

int glms_emit_init(GLMSEmit *emit, GLMSEnv* env, GLMSEmitConfig cfg) {
  if (emit->initialized) return 1;
  if (cfg.mode == GLMS_EMIT_MODE_UNDEFINED) return 0;
  if (env == 0) return 0;

  emit->env = env;

  emit->config = cfg;
  glms_string_builder_destroy(&emit->builder);

  emit->initialized = true;

  return 1;
}

int glms_emit_destroy(GLMSEmit *emit) {
  if (!emit || !emit->initialized) return 0;
  glms_string_builder_destroy(&emit->builder);
  emit->initialized = false;
  return 1;
}

int glms_emit(GLMSEmit *emit, GLMSAST *ast) {
  if (!emit || !ast || emit->config.mode == GLMS_EMIT_MODE_UNDEFINED) return 0;

  switch (emit->config.mode) {
  case GLMS_EMIT_MODE_GLSL: if (!glms_emit_glsl(emit, ast)) return 0; break;
  default: { GLMS_WARNING_RETURN(0, stderr, "Invalid emit mode.\n"); }; break;
  }


  if (!emit->builder.buffer || emit->builder.length <= 0) return 0;
  const char* path = emit->config.out_path;
  if (!path) return 1;


  FILE* fp = fopen(path, "w+");

  if (!fp) GLMS_WARNING_RETURN(0, stderr, "Failed to open `%s`.\n", path);

  fwrite(&emit->builder.buffer[0], sizeof(char), emit->builder.length, fp);

  const char* newline = "\n";
  fwrite(&newline[0], sizeof(char), 1, fp);

  fclose(fp);

  
  return 1;
}
