#include <glms/glms.h>
#include <glms/macros.h>
#include <glms/io.h>


int main(int argc, char* argv[]) {
  if (argc < 2) return 0;

  char* source = glms_get_file_contents(argv[1]);
  if (!source) return 0;

  GLMSEnv env = {0};

  glms_env_init(&env, source, (GLMSConfig){});

  glms_env_exec(&env);

  return 0;
}
