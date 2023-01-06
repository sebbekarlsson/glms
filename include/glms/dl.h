#ifndef GLMS_DL_H
#define GLMS_DL_H
#include <glms/env.h>

void *glms_load_symbol(const char *filename, const char *funcname);
GLMSExtensionEntryFunc glms_load_symbol_function(const char *filename,
                                                 const char *funcname);

#endif
