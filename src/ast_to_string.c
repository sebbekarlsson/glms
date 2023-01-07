#include <glms/ast.h>
#include <glms/env.h>
#include <glms/macros.h>
#include <string.h>
#include <text/text.h>

#include "glms/ast_type.h"
#include "glms/string_view.h"

char* glms_ast_to_string(GLMSAST ast, GLMSAllocator alloc,
                         struct GLMS_ENV_STRUCT* env) {
  GLMSAST* t = glms_env_get_type_for(env, &ast);

  if (t && t->to_string) {
    return t->to_string(&ast, alloc, env);
  }

  if (!alloc.strdup) GLMS_WARNING_RETURN(0, stderr, "No allocator provided.\n");
  if (ast.to_string) {
    return ast.to_string(&ast, alloc, env);
  }

  switch (ast.type) {
    case GLMS_AST_TYPE_BOOL: {
      return alloc.strdup(alloc.user_ptr, ast.as.boolean ? "true" : "false");
    }; break;
    case GLMS_AST_TYPE_STACK_PTR: {
      GLMSAST* ptr = 0;
      if ((ptr = glms_ast_get_ptr(ast))) {
        return glms_ast_to_string(*ptr, alloc, env);
      }
    }; break;
    case GLMS_AST_TYPE_STACK: {
      char* s = 0;

      HashyIterator it = {0};
      while (hashy_map_iterate(&ast.as.stack.env->types, &it)) {
        if (!it.bucket->key) continue;
        if (!it.bucket->value) continue;

        const char* key = it.bucket->key;
        GLMSAST* value = (GLMSAST*)it.bucket->value;

        char* strval = glms_ast_to_string(*value, alloc, env);

        if (!strval) continue;
        alloc.strcat(alloc.user_ptr, &s, key);
        alloc.strcat(alloc.user_ptr, &s, " => ");
        alloc.strcat(alloc.user_ptr, &s, strval);
        alloc.strcat(alloc.user_ptr, &s, "\n");
      }

      HashyIterator it2 = {0};
      while (hashy_map_iterate(&ast.as.stack.env->globals, &it2)) {
        if (!it2.bucket->key) continue;
        if (!it2.bucket->value) continue;

        const char* key = it2.bucket->key;
        GLMSAST* value = (GLMSAST*)it2.bucket->value;

        char* strval = glms_ast_to_string(*value, alloc, env);

        if (!strval) continue;
        alloc.strcat(alloc.user_ptr, &s, key);
        alloc.strcat(alloc.user_ptr, &s, " => ");
        alloc.strcat(alloc.user_ptr, &s, strval);
        alloc.strcat(alloc.user_ptr, &s, "\n");
      }

      if (!s) return strdup(GLMS_AST_TYPE_STR[ast.type]);

      return s;

    }; break;
    case GLMS_AST_TYPE_STRUCT: {
      HashyIterator it = {0};
      char* s = 0;

      if (ast.props.initialized) {
        while (hashy_map_iterate(&ast.props, &it)) {
          if (!it.bucket->key) continue;
          if (!it.bucket->value) continue;

          const char* key = it.bucket->key;
          GLMSAST* value = (GLMSAST*)it.bucket->value;

          char* strval = glms_ast_to_string(*value, alloc, env);

          if (!strval) continue;
          alloc.strcat(alloc.user_ptr, &s, key);
          alloc.strcat(alloc.user_ptr, &s, " => ");
          alloc.strcat(alloc.user_ptr, &s, strval);
          alloc.strcat(alloc.user_ptr, &s, "\n");
        }
      }

      if (s) return s;

      return alloc.strdup(alloc.user_ptr, GLMS_AST_TYPE_STR[ast.type]);
    }; break;
    case GLMS_AST_TYPE_STRING: {
      const char* v = glms_string_view_get_value(&ast.as.string.value);
      if (v != 0) {
        return alloc.strdup(alloc.user_ptr, v);
      }
    }; break;
    case GLMS_AST_TYPE_NUMBER: {
      char tmp[256];
      sprintf(tmp, "%1.6f", glms_ast_number(ast));
      return alloc.strdup(alloc.user_ptr, tmp);
    }; break;
    default: {
      if (ast.json != 0) {
        char* jsonstr = json_stringify(ast.json);
        if (jsonstr) return jsonstr;
      }
      return alloc.strdup(alloc.user_ptr, GLMS_AST_TYPE_STR[ast.type]);
    }; break;
  }

  return 0;
}

char* glms_ast_to_string_debug_binop(GLMSAST ast) {
  char* s = 0;
  text_append(&s, glms_ast_to_string_debug(*ast.as.binop.left));
  text_append(&s, " ");
  text_append(&s, GLMS_TOKEN_TYPE_STR[ast.as.binop.op]);
  text_append(&s, " ");
  text_append(&s, glms_ast_to_string_debug(*ast.as.binop.right));
  return s;
}
char* glms_ast_to_string_debug_unop(GLMSAST ast) {
  char* s = 0;

  if (ast.as.unop.left) {
    text_append(&s, glms_ast_to_string_debug(*ast.as.unop.left));
  }
  text_append(&s, GLMS_TOKEN_TYPE_STR[ast.as.unop.op]);

  if (ast.as.unop.right) {
    text_append(&s, glms_ast_to_string_debug(*ast.as.unop.right));
  }
  return s;
}
char* glms_ast_to_string_debug_id(GLMSAST ast) {
  const char* strval = glms_string_view_get_value(&ast.as.id.value);
  return strdup(OR(strval, "(null)"));
}
char* glms_ast_to_string_debug_string(GLMSAST ast) {
  const char* strval = glms_string_view_get_value(&ast.as.string.value);
  return strdup(OR(strval, "(null)"));
}
char* glms_ast_to_string_debug_number(GLMSAST ast) {
  char tmp[256];
  sprintf(tmp, "%1.6f", glms_ast_number(ast));
  return strdup(tmp);
}

static char* glms_ast_to_string_debug_(GLMSAST ast) {
  switch (ast.type) {
    case GLMS_AST_TYPE_BINOP: {
      return glms_ast_to_string_debug_binop(ast);
    }; break;
    case GLMS_AST_TYPE_UNOP: {
      return glms_ast_to_string_debug_unop(ast);
    }; break;
    case GLMS_AST_TYPE_ID: {
      return glms_ast_to_string_debug_id(ast);
    }; break;
    case GLMS_AST_TYPE_STRING: {
      return glms_ast_to_string_debug_string(ast);
    }; break;
    case GLMS_AST_TYPE_NUMBER: {
      return glms_ast_to_string_debug_number(ast);
    }; break;
    default: {
      return strdup(GLMS_AST_TYPE_STR[ast.type]);
    }; break;
  }
}
char* glms_ast_to_string_debug(GLMSAST ast) {
  char* ast_str = glms_ast_to_string_debug_(ast);

  char* s = 0;
  text_append(&s, GLMS_AST_TYPE_STR[ast.type]);
  text_append(&s, "(\n");
  text_append(&s, ast_str ? ast_str : "?");
  text_append(&s, "\n)");

  return s;
}
