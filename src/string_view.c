#include <jscript/string_view.h>
#include <jscript/macros.h>
#include <string.h>

const char* jscript_string_view_get_value(JSCRIPTStringView* view) {
  if (!view) return 0;
  if (view->length <= 0 || view->ptr == 0) return 0;
  if (view->length >= JSCRIPT_STRING_VIEW_CAPACITY) JSCRIPT_WARNING_RETURN(0, stderr, "string too large.\n");
  memset(&view->tmp_buffer[0], 0, JSCRIPT_STRING_VIEW_CAPACITY *sizeof(char));
  memcpy(&view->tmp_buffer[0], &view->ptr[0], MIN(view->length, (JSCRIPT_STRING_VIEW_CAPACITY-1)));
  return view->tmp_buffer;
}
