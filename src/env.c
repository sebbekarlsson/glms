#include "arena/arena.h"
#include "arena/config.h"
#include "glms/allocator.h"
#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/eval.h"
#include "glms/stack.h"
#include "glms/type.h"
#include "hashy/hashy.h"
#include "text/text.h"
#include <glms/builtin.h>
#include <glms/constants.h>
#include <glms/env.h>
#include <glms/io.h>
#include <glms/macros.h>
#include <spath/spath.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

int glms_env_init(GLMSEnv *env, const char *source, const char* entry_path, GLMSConfig cfg) {
  if (!env)
    return 0;
  if (env->initialized)
    return 1;
  env->initialized = true;
  env->config = cfg;
  env->source = source;
  env->entry_path = entry_path;
  env->last_joined_path = 0;
  glms_allocator_string_allocator(&env->string_alloc);
  hashy_map_init(&env->globals, 256);
  hashy_map_init(&env->types, 256);
  memo_init(
      &env->memo_ast,
      (MemoConfig){.item_size = sizeof(GLMSAST),
		   .page_capacity = GLMS_MEMO_AST_PAGE_CAPACITY,
		   .destructor = (MemoDestructorFunc)glms_ast_destructor});

  arena_init(
      &env->arena_ast,
      (ArenaConfig){.items_per_page = GLMS_ARENA_AST_CAPACITY,
		    .item_size = sizeof(GLMSAST),
		    .free_function = (ArenaFreeFunction)glms_ast_destructor});

  env->undefined = glms_env_new_ast(env, GLMS_AST_TYPE_UNDEFINED, false);
  env->stackptr = glms_env_new_ast(env, GLMS_AST_TYPE_STACK_PTR, false);

  glms_eval_init(&env->eval, env);
  glms_stack_init(&env->stack);
  glms_builtin_init(env);

  if (env->source != 0) {
    glms_lexer_init(&env->lexer, env->source);
    glms_parser_init(&env->parser, env);
  }

  return 1;
}

int glms_env_clear(GLMSEnv *env) {
  if (!env)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  env->source = 0;
  hashy_map_clear(&env->parser.symbols, false);
  hashy_map_clear(&env->globals, false);
  hashy_map_clear(&env->types, false);
  glms_stack_clear(&env->stack);
  env->undefined = 0;
  memo_clear(&env->memo_ast);

  arena_destroy(&env->arena_ast);
  //arena_reset(&env->arena_ast);
  //arena_clear(&env->arena_ast);

  env->initialized = false;

  return 1;
}

GLMSAST *glms_env_new_ast(GLMSEnv *env, GLMSASTType type, bool arena) {
  if (!env)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  arena = env->use_arena;

  ArenaRef ref = {0};
  GLMSAST *ast = 0;
  ast = (GLMSAST *)(arena ? arena_malloc(&env->arena_ast, &ref)
			  : memo_malloc(&env->memo_ast));
  if (!ast)
    GLMS_WARNING_RETURN(0, stderr, "Failed to allocate AST.\n");

  ast->type = type;
  ast->ref = ref;
  ast->is_heap = true;

  #if 0
  if (env->arena_ast.pages >= GLMS_GC_SWEEP_THRESHOLD) {
    for (int i = 0; i < GLMS_GC_SWEEP_ITER; i++) {
       arena_defrag(&env->arena_ast);
    }
  }
  #endif

  return ast;
}

GLMSAST *glms_env_new_ast_number(GLMSEnv *env, float v, bool arena) {
  GLMSAST *ast = glms_env_new_ast(env, GLMS_AST_TYPE_NUMBER, arena);
  ast->as.number.value = v;
  return ast;
}

GLMSAST *glms_env_new_ast_vec3(GLMSEnv *env, Vector3 v, bool arena) {
GLMSAST *ast = glms_env_new_ast(env, GLMS_AST_TYPE_VEC3, arena);
  ast->as.v3 = v;
  return ast;
}

GLMSAST *glms_env_new_ast_make(GLMSEnv *env, GLMSAST ast, bool arena) {
  GLMSAST *new_ast = glms_env_new_ast(env, ast.type, arena);
  *new_ast = ast;
  return new_ast;
}

GLMSAST *glms_env_new_ast_field(GLMSEnv *env, GLMSTokenType data_type,
				const char *name, bool arena) {
  GLMSAST *ast = glms_env_new_ast(env, GLMS_AST_TYPE_ID, arena);
  ast->as.id.heap = name ? strdup(name) : 0;
  GLMSAST *flag = glms_env_new_ast(env, GLMS_AST_TYPE_ID, arena);
  flag->as.id.op = data_type;
  glms_ast_push_flag(ast, flag);
  return ast;
}

GLMSAST *glms_env_new_ast_string(GLMSEnv *env, const char *value, bool arena) {
  GLMSAST *ast = glms_env_new_ast(env, GLMS_AST_TYPE_STRING, arena);

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

  env->use_arena = false;
  
  GLMSAST *root = glms_parser_parse(&env->parser);

  env->use_arena = true;

  glms_eval(&env->eval, *root, &env->stack);
  return root;
}

int glms_env_register_function_signature(GLMSEnv *env, GLMSAST* ast, const char *name,
                                          GLMSFunctionSignature signature) {

 if (!env || !name)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  GLMSAST* func =  0;

  if (ast != 0) {
    func = glms_ast_access_by_key(ast, name, env);
  } else {
    func = glms_env_lookup_function(env, name);
  }

  if (!func) return 0;
  if (func->type != GLMS_AST_TYPE_FUNC) return 0;

  if (func->as.func.signatures.initialized == false) {
    glms_GLMSFunctionSignature_buffer_init(&func->as.func.signatures);
  }

GLMSFunctionSignature next = {0};
next.args_length = signature.args_length;
next.return_type = signature.return_type;

 if (signature.description) next.description = strdup(signature.description);

if (signature.args && signature.args_length) {
    next.args = (GLMSType*)calloc(signature.args_length, sizeof(GLMSType));

    for (int i = 0; i < signature.args_length; i++) {
      next.args[i] = signature.args[i];

      if (signature.args[i].valuename != 0) {
	next.args[i].valuename = strdup(signature.args[i].valuename);
      }

       if (signature.args[i].typename != 0) {
	next.args[i].typename = strdup(signature.args[i].typename);
      }
    }
}

glms_GLMSFunctionSignature_buffer_push(&func->as.func.signatures, next);
  
  return 1;
}

GLMSAST *glms_env_register_function(GLMSEnv *env, const char *name,
				    GLMSFPTR fptr) {
  if (!env || !name || !fptr)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  GLMSAST *func = glms_env_new_ast(env, GLMS_AST_TYPE_FUNC, false);
  func->fptr = fptr;
  func->as.func.name = strdup(name);
  hashy_map_set(&env->globals, name, func);

  return func;
}

GLMSAST *glms_env_register_struct(GLMSEnv *env, const char *name,
				  GLMSAST **fields, int fields_length) {
  if (!env || !name || !fields)
    return 0;
  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  GLMSAST *tdef = glms_env_new_ast(env, GLMS_AST_TYPE_TYPEDEF, false);
  GLMSAST *stru = glms_env_new_ast(env, GLMS_AST_TYPE_STRUCT, false);
  tdef->as.tdef.factor = stru;
  tdef->as.tdef.id = glms_env_new_ast(env, GLMS_AST_TYPE_ID, false);
  tdef->as.tdef.id->as.id.heap = strdup(name);

  for (int i = 0; i < fields_length; i++) {
    GLMSAST *field = fields[i];
    const char *field_name = glms_ast_get_name(field);
    if (!field) {
      GLMS_WARNING(stderr, "Expected a name to exist.\n");
      continue;
    }

    glms_ast_object_set_property(stru, field_name, field);
  }

  hashy_map_set(&env->globals, name, stru);

  return tdef;
}

GLMSAST *glms_env_register_type(GLMSEnv *env, const char *name, GLMSAST *ast,
				GLMSASTContructor constructor,
				GLMSASTSwizzle swizzle,
				GLMSASTToString to_string, GLMSASTDestructor destructor) {
  if (!env || !name || !ast)
    return 0;
  ast->constructor = constructor;
  ast->swizzle = swizzle;
  ast->to_string = to_string;
  ast->destructor = destructor;

  if (!ast->typename) ast->typename = strdup(name);
  
  hashy_map_set(&env->globals, name, ast);

  const char *typename = GLMS_AST_TYPE_STR[ast->type];

  //hashy_map_set(&env->types, typename, ast);
  hashy_map_set(&env->types, name, ast);

  return ast;
}
GLMSAST *glms_env_register_any(GLMSEnv *env, const char *name, GLMSAST *ast) {
  if (!name || !ast || !env)
    return 0;
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

GLMSAST *glms_env_lookup_type(GLMSEnv *env, const char *name) {
  if (!env || !name)
    return 0;

  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

  return (GLMSAST *)hashy_map_get(&env->types, name);
}

GLMSAST* glms_env_apply_type(GLMSEnv* env, GLMSEval* eval, GLMSStack* stack, GLMSAST* ast) {
  if (!env || !ast)
    return 0;

  if (ast->type == GLMS_AST_TYPE_STACK_PTR) {
    GLMSAST* ptr = glms_ast_get_ptr(*ast);

    if (ptr) return glms_env_apply_type(env, eval, stack, ptr);
  }

  if (ast->type == GLMS_AST_TYPE_BINOP) return ast;
  //  if (ast->value_type != 0 && ast->constructor != 0) return ast;
  if (ast->constructed) return ast;

  GLMSAST *type = ast->value_type;


  if (type == 0 && ast->typename) {
    type = glms_env_lookup_type(env, ast->typename);
  }
  
  type = type ? type : glms_env_lookup_type(env, glms_ast_get_name(ast));
  type = type ? type : glms_env_lookup_type(env, GLMS_AST_TYPE_STR[ast->type]);

  if (!type)
    return 0;

  ast->constructor = type->constructor;


  if (type->constructor) {
    GLMSASTBuffer args = {0};
    glms_GLMSAST_buffer_init(&args);

    if (ast->children) {
      for (int64_t i = 0; i < ast->children->length; i++) {
	glms_GLMSAST_buffer_push(&args, glms_eval(eval, *ast->children->items[i], stack));
      }
    }
    type->constructor(eval, stack, &args, ast);
    ast->constructed = true;

    glms_GLMSAST_buffer_clear(&args);
  }
  //ast->to_string = type->to_string;
  // ast->swizzle = type->swizzle;

  /*
  if (type->props.initialized) {
    HashyIterator it = {0};

    while (hashy_map_iterate(&type->props, &it)) {
      if (!it.bucket->key)
	continue;
      if (!it.bucket->value)
	continue;

      const char *key = it.bucket->key;
      GLMSAST *value = (GLMSAST *)it.bucket->value;

      glms_ast_object_set_property(ast, key, value);
    }
   }*/

  ast->value_type = type;

  return ast;
}

const char* glms_env_get_path_for(GLMSEnv* env, const char* path) {
  if (!env->entry_path) {
    return path;
  }

  const char* basepath = env->entry_path;
  char dirname[PATH_MAX];

  if (spath_get_dirname(basepath, dirname)) {
    basepath = dirname;
  }
  
  char* joined = spath_join(basepath, path);

  if (!joined) return path;

  if (env->last_joined_path != 0) {
    free(env->last_joined_path);
    env->last_joined_path = 0;
  }

  env->last_joined_path = strdup(joined);

  return env->last_joined_path;
}


char* glms_env_export_docstrings_from_map(GLMSEnv *env, HashyMap map, bool only_functions) {

   char* str = 0;
  HashyIterator it = {0};


  if (!map.initialized) return 0;

  GLMSDocstringGenerator gen = {0};
    while (hashy_map_iterate(&map, &it)) {
      if (!it.bucket->key)
	continue;
      if (!it.bucket->value)
	continue;

      const char *key = it.bucket->key;
      GLMSAST *value = (GLMSAST *)it.bucket->value;
      if (key[0] == '_') continue;


      if (only_functions && value->type != GLMS_AST_TYPE_FUNC) continue;

      glms_env_apply_type(env, &env->eval, &env->stack, value);

      char* signature_str = glms_ast_generate_docstring(*value, key, 0, 0, &gen);

      if (!signature_str) continue;

      text_append(&str, signature_str);
      text_append(&str, "\n");
    }

    if (!str) return 0;

    return str;
}

int glms_env_export_docstrings(GLMSEnv* env, const char* filepath) {
    if (!env || !filepath)
    return 0;

  if (!env->initialized)
    GLMS_WARNING_RETURN(0, stderr, "env not initialized.\n");

    char* str = 0;

    char* str0 = 0;
    if ((str0 = glms_env_export_docstrings_from_map(env, env->globals, true))) { text_append(&str, str0); }
    if ((str0 = glms_env_export_docstrings_from_map(env, env->types, false))) { text_append(&str, str0); }

    FILE* fp = fopen(filepath, "w+");

    if (!fp) GLMS_WARNING_RETURN(0, stderr, "Failed to open `%s`\n", filepath);

    fwrite(&str[0], sizeof(char), strlen(str), fp);

    fclose(fp);

    return 1;
}
