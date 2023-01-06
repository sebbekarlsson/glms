#include <glms/glms.h>
#include <glms/io.h>
#include <glms/macros.h>
#include <glms/version.h>
#include <hashy/hashy.h>
#include <stdio.h>
#include <string.h>

#include "glms/env.h"

typedef struct {
  union {
    bool boolean;
  } as;
} CLIArg;

typedef struct {
  HashyMap args;
  bool initialized;
} CLIArgs;

int cli_args_init(CLIArgs* args, int argc, char* argv[]) {
  if (args->initialized) return 1;
  args->initialized = true;
  hashy_map_init(&args->args, 16);

  for (int i = 0; i < argc; i++) {
    CLIArg* carg = NEW(CLIArg);
    hashy_map_set(&args->args, argv[i], carg);
  }

  return 1;
}

bool cli_args_has(CLIArgs* args, const char* key) {
  if (!args->initialized) return false;
  return hashy_map_get(&args->args, key) != 0;
}

int cli_args_destroy(CLIArgs* args) {
  if (!args->initialized) return 0;
  hashy_map_clear(&args->args, true);
  return 1;
}

static int glms_interactive() {
  GLMSEnv env = {0};
  glms_env_init(&env, 0, 0, (GLMSConfig){});
  while (true) {
    char input_source[1024];
    memset(&input_source[0], 0, 1024 * sizeof(char));

    printf("> ");
    fgets(input_source, 1024, stdin);
    fflush(stdin);

    glms_env_exec_source(&env, input_source);
    glms_env_reset(&env);
  }

  glms_env_clear(&env);
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2) return glms_interactive();

  CLIArgs cli = {0};
  cli_args_init(&cli, argc, argv);

  if (cli_args_has(&cli, "--export-docs")) {
    GLMSEnv env = {0};
    glms_env_init(&env, 0, argv[1], (GLMSConfig){});
    glms_env_export_docstrings(&env, "signatures.md");
    glms_env_clear(&env);
    cli_args_destroy(&cli);
    return 0;
  } else if (cli_args_has(&cli, "--version")) {
    printf("GLMS Version %s\n", GLMS_VERSION_STRING);
    return 0;
  }

  char* source = glms_get_file_contents(argv[1]);
  if (!source) {
    cli_args_destroy(&cli);
    return 0;
  }

  GLMSEnv env = {0};
  glms_env_init(&env, source, argv[1], (GLMSConfig){});
  glms_env_exec(&env);
  glms_env_clear(&env);

  free(source);
  source = 0;

  cli_args_destroy(&cli);
  return 0;
}
