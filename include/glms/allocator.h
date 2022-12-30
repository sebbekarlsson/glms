#ifndef GLMS_ALLOCATOR_H
#define GLMS_ALLOCATOR_H
#include <stdint.h>

typedef void *(*GLMSAllocatorFunc)(void *user_ptr, int64_t size);
typedef char *(*GLMSAllocatorStrdup)(void *user_ptr, const char* str);

typedef struct {
  void* user_ptr;
  GLMSAllocatorFunc func;
  GLMSAllocatorStrdup strdup;
} GLMSAllocator;


void glms_allocator_string_allocator(GLMSAllocator* alloc);
#endif
