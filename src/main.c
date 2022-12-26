#include <glms/glms.h>
#include <glms/io.h>
#include <glms/macros.h>

int main(int argc, char *argv[]) {
  if (argc < 2)
    return 0;

  char *source = glms_get_file_contents(argv[1]);
  if (!source)
    return 0;

  GLMSEnv env = {0};

  glms_env_init(&env, source, (GLMSConfig){});
  glms_env_exec(&env);

  glms_env_clear(&env);

  free(source);
  source = 0;

  return 0;
}
