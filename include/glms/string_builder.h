#ifndef GLMS_STRING_BUILDER_H
#define GLMS_STRING_BUILDER_H
#include <stdint.h>
typedef struct {
  char* buffer;
  int64_t length;
  int64_t capacity;
} GLMSStringBuilder;

int glms_string_builder_append(GLMSStringBuilder *builder, const char *value);

int glms_string_builder_append_indented(GLMSStringBuilder *builder, const char *value, int indent, const char* indent_v);

int glms_string_builder_destroy(GLMSStringBuilder* builder);
#endif
