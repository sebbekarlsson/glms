#include <glms/glms.h>
#include <glms/io.h>
#include <glms/macros.h>
#include <glms/version.h>
#include <hashy/hashy.h>
#include <stdio.h>
#include <string.h>
#include <glms/string_builder.h>

#include "glms/emit/emit.h"
#include "glms/env.h"

typedef enum {
  CLI_ARG_BOOLEAN,
  CLI_ARG_STRING
} CLIArgType;

typedef struct {
  union {
    bool boolean;
    const char* str;
  } as;
  CLIArgType type;
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

    const char* key = argv[i];
    int64_t len = strlen(key);
    if (len > 2 && ((i+1) < argc) && key[0] == '-' && key[1] == '-') {
      const char* value = argv[i+1];
      carg->type = CLI_ARG_STRING;
      carg->as.str = value;
      hashy_map_set(&args->args, argv[i], carg);
      i++;
      continue;
    }
    
    carg->type = CLI_ARG_BOOLEAN;
    hashy_map_set(&args->args, argv[i], carg);
  }

  return 1;
}

bool cli_args_has(CLIArgs* args, const char* key) {
  if (!args->initialized) return false;
  return hashy_map_get(&args->args, key) != 0;
}

const char* cli_args_get_string(CLIArgs* args, const char* key) {
  if (!args->initialized) return false;
  CLIArg* arg = (CLIArg*)hashy_map_get(&args->args, key);
  if (!arg || arg->type != CLI_ARG_STRING) return 0;
  return arg->as.str;
}

int cli_args_destroy(CLIArgs* args) {
  if (!args->initialized) return 0;
  hashy_map_clear(&args->args, true);
  return 1;
}

static int glms_interactive(GLMSConfig cfg) {
  GLMSEnv env = {0};
  glms_env_init(&env, 0, 0, cfg);
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
  GLMSConfig cfg = {0};
  if (argc < 2) return glms_interactive(cfg);

  CLIArgs cli = {0};
  cli_args_init(&cli, argc, argv);

  if (cli_args_has(&cli, "--export-docs")) {
    GLMSEnv env = {0};
    glms_env_init(&env, 0, argv[1], (GLMSConfig){0});
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

  if (cli_args_has(&cli, "--emit")) {
    cfg.emit.out_path = cli_args_get_string(&cli, "--emit");
    cfg.emit.mode = GLMS_EMIT_MODE_GLSL; // TODO: add support for other emit modes.
  }

  GLMSEnv env = {0};
  glms_env_init(&env, source, argv[1], cfg);
  glms_env_exec(&env);
  glms_env_clear(&env);

  free(source);
  source = 0;

  cli_args_destroy(&cli);
  return 0;
}
