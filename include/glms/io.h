#ifndef GLMS_IO_H
#define GLMS_IO_H
#include <stdbool.h>
bool glms_file_exists(const char *path);
char *glms_get_file_contents(const char *filepath);
#endif
