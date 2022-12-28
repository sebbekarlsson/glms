#include <glms/builtin.h>
#include <glms/constants.h>
#include <glms/env.h>
#include <glms/io.h>
#include <glms/macros.h>

int glms_env_init(GLMSEnv *env, const char *source, GLMSConfig cfg) {
  if (!env)
    return 0;
  if (env->initialized)
    return 1;
  env->initialized = true;
  env->config = cfg;
  env->source = source;
  hashy_map_init(&env->globals, 256);
  memo_init(&env->memo_ast,
            (MemoConfig){.item_size = sizeof(GLMSAST),
            .page_capacity = GLMS_MEMO_AST_PAGE_CAPACITY, .destructor = (MemoDestructorFunc)glms_ast_destructor});

  env->undefined = glms_env_new_ast(env, GLMS_AST_TYPE_UNDEFINED);

  glms_eval_init(&env->eval, env);
  glms_stack_init(&env->stack);
  glms_builtin_init(env);
  glms_lexer_init(&env->lexer, env->source);
  glms_parser_init(&env->parser, env);


  return 1;
}


int glms_env_clear(GLMSEnv* env) {
  if (!env) return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");


  env->source = 0;
  hashy_map_clear(&env->parser.symbols, false);
  hashy_map_clear(&env->globals, false);
  glms_stack_clear(&env->stack);
  env->undefined = 0;
  memo_clear(&env->memo_ast);

  env->initialized = false;

  return 1;
}

GLMSAST *glms_env_new_ast(GLMSEnv *env, GLMSASTType type) {
  if (!env)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  GLMSAST *ast = (GLMSAST *)memo_malloc(&env->memo_ast);
  if (!ast)
    GLMS_WARNING_RETURN(0, stderr, "Failed to allocate AST.\n");


  ast->type = type;
  return ast;
}

GLMSAST *glms_env_new_ast_number(GLMSEnv *env, float v) {
  GLMSAST *ast = glms_env_new_ast(env, GLMS_AST_TYPE_NUMBER);
  ast->as.number.value = v;
  return ast;
}

GLMSAST* glms_env_new_ast_make(GLMSEnv* env, GLMSAST ast) {
  GLMSAST *new_ast = glms_env_new_ast(env, ast.type);
  *new_ast = ast;
  return new_ast;
}

GLMSAST* glms_env_new_ast_field(GLMSEnv* env, GLMSTokenType data_type, const char* name) {
  GLMSAST *ast = glms_env_new_ast(env, GLMS_AST_TYPE_ID);
  ast->as.id.heap = name ? strdup(name) : 0;
  GLMSAST* flag = glms_env_new_ast(env, GLMS_AST_TYPE_ID);
  flag->as.id.op = data_type;
  glms_ast_push_flag(ast, flag);
  return ast;
}

GLMSAST *glms_env_new_ast_string(GLMSEnv *env, const char *value) {
  GLMSAST *ast = glms_env_new_ast(env, GLMS_AST_TYPE_STRING);

  if (value != 0) {
    ast->as.string.heap = strdup(value);
  }

  return ast;
}

GLMSAST *glms_env_exec(GLMSEnv *env) {
  if (!env)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  GLMSAST *root = glms_parser_parse(&env->parser);

  root = glms_eval(&env->eval, root, &env->stack);
  return root;
}

GLMSAST *glms_env_register_function(GLMSEnv *env, const char *name,
                                    GLMSFPTR fptr) {
  if (!env || !name || !fptr)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  GLMSAST *func = glms_env_new_ast(env, GLMS_AST_TYPE_FUNC);
  func->fptr = fptr;
  hashy_map_set(&env->globals, name, func);

  return func;
}

GLMSAST *glms_env_register_struct(GLMSEnv *env, const char *name,
                                  GLMSAST** fields, int fields_length) {
  if (!env || !name || !fields)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  GLMSAST *tdef = glms_env_new_ast(env, GLMS_AST_TYPE_TYPEDEF);
  GLMSAST *stru = glms_env_new_ast(env, GLMS_AST_TYPE_STRUCT);
  tdef->as.tdef.factor = stru;
  tdef->as.tdef.id = glms_env_new_ast(env, GLMS_AST_TYPE_ID);
  tdef->as.tdef.id->as.id.heap = strdup(name);

  for (int i = 0; i < fields_length; i++) {
    GLMSAST* field = fields[i];
    const char* field_name = glms_ast_get_name(field);
    if (!field) {
      GLMS_WARNING(stderr, "Expected a name to exist.\n");
      continue;
    }

    glms_ast_object_set_property(stru, field_name, field);
  }


  hashy_map_set(&env->globals, name, stru);

  return tdef;
}

GLMSAST *glms_env_register_type(
				GLMSEnv *env,
				const char *name,
				GLMSAST* ast,
				GLMSASTContructor constructor,
				GLMSASTSwizzle swizzle,
				GLMSASTToString to_string
				) {
  if (!env || !name || !ast) return 0;
  ast->constructor = constructor;
  ast->swizzle = swizzle;
  ast->to_string = to_string;
  hashy_map_set(&env->globals, name, ast);
  return ast;
}
GLMSAST *glms_env_register_any(GLMSEnv *env, const char *name, GLMSAST *ast) {
  if (!name || !ast || !env) return 0;
  hashy_map_set(&env->globals, name, ast);
  return ast;
}

GLMSAST *glms_env_lookup_function(GLMSEnv *env, const char *name) {
  if (!env || !name)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  return (GLMSAST *)hashy_map_get(&env->globals, name);
}
