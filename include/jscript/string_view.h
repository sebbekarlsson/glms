#ifndef JSCRIPT_STRING_VIEW_H
#define JSCRIPT_STRING_VIEW_H
#include <stdint.h>

#define JSCRIPT_STRING_VIEW_CAPACITY 256

typedef struct {
  int64_t length;
  const char* ptr;
  char tmp_buffer[JSCRIPT_STRING_VIEW_CAPACITY];
} JSCRIPTStringView;


const char* jscript_string_view_get_value(JSCRIPTStringView* view);

#endif
