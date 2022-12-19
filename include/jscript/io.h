#ifndef JSCRIPT_IO_H
#define JSCRIPT_IO_H
#include <stdbool.h>
bool jscript_file_exists(const char *path);
char *jscript_get_file_contents(const char *filepath);
#endif
