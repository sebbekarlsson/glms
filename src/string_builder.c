#include <glms/string_builder.h>
#include <glms/macros.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int glms_string_builder_append(GLMSStringBuilder *builder, const char *value) {
  if (!builder || !value) return 0;

  int64_t len = strlen(value);
  if (len <= 0) return 0;


  if(builder->capacity < builder->length + len + 1) {
    builder->buffer = (char*)realloc(builder->buffer, (builder->length + len + 1) * sizeof(char));
    if (!builder->buffer) {
      GLMS_WARNING_RETURN(0, stderr, "Allocation failure.\n");
    }
    builder->capacity = builder->length + len + 1;
  }

  strcpy(builder->buffer + builder->length, value);

  builder->length += len;

  return 1;
}

int glms_string_builder_append_indented(GLMSStringBuilder *builder,
                                        const char *value, int indent,
                                        const char *indent_v) {
  if (!builder || !value) return 0;
  if (!indent || !indent_v) return glms_string_builder_append(builder, value);

  for (int i = 0; i < indent; i++) {
    if (!glms_string_builder_append(builder, indent_v)) return 0;
  }

  return glms_string_builder_append(builder, value);
}

int glms_string_builder_destroy(GLMSStringBuilder *builder) {
  if (!builder) return 0;

  if (builder->buffer != 0) {
    free(builder->buffer);
  }
  
  builder->buffer = 0;
  builder->length = 0;
  
  return 1;
}
