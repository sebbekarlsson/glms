#ifndef GLMS_ALLOCATOR_H
#define GLMS_ALLOCATOR_H
#include <stdint.h>

typedef void* (*GLMSAllocatorFunc)(void* user_ptr, int64_t size);
typedef char* (*GLMSAllocatorStrdup)(void* user_ptr, const char* str);

typedef void (*GLMSAllocatorStrcat)(void* user_ptr, char** instr,
                                    const char* append);

typedef struct {
  void* user_ptr;
  GLMSAllocatorFunc func;
  GLMSAllocatorStrdup strdup;
  GLMSAllocatorStrcat strcat;
} GLMSAllocator;

void glms_allocator_string_allocator(GLMSAllocator* alloc);
#endif
