#include <jscript/jscript.h>
#include <jscript/macros.h>
#include <jscript/io.h>


int main(int argc, char* argv[]) {
  if (argc < 2) return 0;

  char* source = jscript_get_file_contents(argv[1]);
  if (!source) return 0;

  JSCRIPTEnv env = {0};

  jscript_env_init(&env, source, (JSCRIPTConfig){});

  jscript_env_exec(&env);

  return 0;
}
