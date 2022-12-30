#include <glms/allocator.h>
#include <glms/macros.h>
#include <string.h>
#include <stdlib.h>



char *glms_allocator_string_allocator_strdup(void *user_ptr, const char *s) {
  return strdup(s);
}

void glms_allocator_string_allocator(GLMSAllocator *alloc) {
  if (!alloc) return;
  alloc->strdup = glms_allocator_string_allocator_strdup;
  alloc->func = 0;
}
