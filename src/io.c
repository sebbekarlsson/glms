#include <glms/io.h>
#include <glms/macros.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

bool glms_file_exists(const char* path) { return access(path, F_OK) == 0; }

char* glms_get_file_contents(const char* filepath) {
  if (!glms_file_exists(filepath)) {
    fprintf(stderr, "Error: No such file %s\n", filepath);
    return 0;
  }
  FILE* fp = fopen(filepath, "r");
  char* buffer = NULL;
  size_t len;
  ssize_t bytes_read = getdelim(&buffer, &len, '\0', fp);
  if (bytes_read == -1) {
    printf("Failed to read file `%s`\n", filepath);
    return 0;
  }

  fclose(fp);

  return buffer;
}
