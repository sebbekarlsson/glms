#ifndef GLMS_STRING_VIEW_H
#define GLMS_STRING_VIEW_H
#include <stdint.h>

#define GLMS_STRING_VIEW_CAPACITY 256

typedef struct {
  int64_t length;
  const char* ptr;
  char tmp_buffer[GLMS_STRING_VIEW_CAPACITY];
} GLMSStringView;

const char* glms_string_view_get_value(GLMSStringView* view);

#endif
