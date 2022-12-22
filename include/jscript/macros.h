#ifndef JSCRIPT_MACROS_H
#define JSCRIPT_MACROS_H
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


#define JSCRIPT_CLI_RED "\x1B[31m"
#define JSCRIPT_CLI_GREEN "\x1B[32m"
#define JSCRIPT_CLI_YELLLOW "\x1B[33m"
#define JSCRIPT_CLI_BLUE "\x1B[34m"
#define JSCRIPT_CLI_MAGENTA "\x1B[35m"
#define JSCRIPT_CLI_CYAN "\x1B[36m"
#define JSCRIPT_CLI_WHITE "\x1B[37m"
#define JSCRIPT_CLI_RESET "\x1B[0m"

#define JSCRIPT_WARNING(...)                                                      \
  {                                                                            \
    printf(JSCRIPT_CLI_RED "(JSCRIPT)(Warning)(%s): \n" JSCRIPT_CLI_RESET, __func__);   \
    fprintf(__VA_ARGS__);                                                      \
  }
#define JSCRIPT_WARNING_RETURN(ret, ...)                                          \
  {                                                                            \
    printf("\n****\n");                                                        \
    printf(JSCRIPT_CLI_RED "(JSCRIPT)(Warning)(%s): \n" JSCRIPT_CLI_RESET, __func__);   \
    fprintf(__VA_ARGS__);                                                      \
    printf("\n****\n");                                                        \
    return ret;                                                                \
  }

#define JSCRIPT_GENERATE_ENUM(ENUM) ENUM,
#define JSCRIPT_GENERATE_STRING(STRING) #STRING,

#endif
