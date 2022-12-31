#include <glms/dl.h>
#include <dlfcn.h>
#include <glms/macros.h>
#include <stdio.h>
#include <stdlib.h>

void *glms_load_symbol(const char *filename, const char *funcname) {
  void *handle;
  char *error;
  void *fptr = 0;

  // /lib/x86_64-linux-gnu/libc.so.6"

  handle = dlopen(filename, RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "%s\n", dlerror());
    exit(1);
  }

  dlerror();

  void *ptr = dlsym(handle, funcname);

  if ((error = dlerror()) != NULL) {
    GLMS_WARNING_RETURN(0, stderr, "%s\n", error);
  }

  return ptr;
}
