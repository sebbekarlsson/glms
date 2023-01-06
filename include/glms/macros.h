#ifndef GLMS_MACROS_H
#define GLMS_MACROS_H
#include <stdio.h>
#include <stdlib.h>

#ifndef NEW
#define NEW(T) ((T *)(calloc(1, sizeof(T))))
#endif

#ifndef OR
#define OR(a, b) (a ? a : b)
#endif

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#ifndef ABS
#define ABS(a) (a < 0 ? (a * -1) : a)
#endif

#define GLMS_CLI_RED "\x1B[31m"
#define GLMS_CLI_GREEN "\x1B[32m"
#define GLMS_CLI_YELLLOW "\x1B[33m"
#define GLMS_CLI_BLUE "\x1B[34m"
#define GLMS_CLI_MAGENTA "\x1B[35m"
#define GLMS_CLI_CYAN "\x1B[36m"
#define GLMS_CLI_WHITE "\x1B[37m"
#define GLMS_CLI_RESET "\x1B[0m"

#define GLMS_WARNING(...)                                                    \
  {                                                                          \
    printf(GLMS_CLI_RED "(GLMS)(Warning)(%s): \n" GLMS_CLI_RESET, __func__); \
    fprintf(__VA_ARGS__);                                                    \
  }
#define GLMS_WARNING_RETURN(ret, ...)                                        \
  {                                                                          \
    printf("\n****\n");                                                      \
    printf(GLMS_CLI_RED "(GLMS)(Warning)(%s): \n" GLMS_CLI_RESET, __func__); \
    fprintf(__VA_ARGS__);                                                    \
    printf("\n****\n");                                                      \
    return ret;                                                              \
  }

#define GLMS_GENERATE_ENUM(ENUM) ENUM,
#define GLMS_GENERATE_STRING(STRING) #STRING,

#endif
