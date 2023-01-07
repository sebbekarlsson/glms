#include <glms/macros.h>
#include <glms/string_view.h>
#include <string.h>

const char* glms_string_view_get_value(GLMSStringView* view) {
  if (!view) return 0;
  if (view->length <= 0 || view->ptr == 0) return 0;
  if (view->length >= GLMS_STRING_VIEW_CAPACITY)
    GLMS_WARNING_RETURN(0, stderr, "string too large.\n");
  memset(&view->tmp_buffer[0], 0, GLMS_STRING_VIEW_CAPACITY * sizeof(char));
  memcpy(&view->tmp_buffer[0], &view->ptr[0],
         sizeof(char) * MIN(view->length, (GLMS_STRING_VIEW_CAPACITY - 1)));
  return view->tmp_buffer;
}
