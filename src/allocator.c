#include <glms/allocator.h>
#include <glms/macros.h>
#include <stdlib.h>
#include <string.h>
#include <text/text.h>

// TODO: use some kind of arena for these strings

char* glms_allocator_string_allocator_strdup(void* user_ptr, const char* s) {
  return strdup(s);
}

void glms_allocator_string_allocator_strcat(void* user_ptr, char** instr,
                                            const char* append) {
  text_append(instr, append);
}

void glms_allocator_string_allocator(GLMSAllocator* alloc) {
  if (!alloc) return;
  alloc->strdup = glms_allocator_string_allocator_strdup;
  alloc->strcat = glms_allocator_string_allocator_strcat;
  alloc->func = 0;
}
