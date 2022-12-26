#include <glms/ast.h>
#include <glms/macros.h>
#include <glms/env.h>
#include <text/text.h>


GLMS_IMPLEMENT_BUFFER(GLMSAST);
GLMS_IMPLEMENT_LIST(GLMSAST);

bool glms_ast_is_iterable(GLMSAST* ast) {
  if (!ast) return false;
  return ast->type == GLMS_AST_TYPE_COMPOUND;
}

GLMSAST* glms_ast_push(GLMSAST* parent, GLMSAST* child) {
  if (!parent || !child) return 0;

  if (!parent->children) {
    parent->children = NEW(GLMSASTList);
  }

  if (!parent->children->initialized) {
    glms_GLMSAST_list_init(parent->children);
  }

  glms_GLMSAST_list_push(parent->children, child);

  return child;
}

GLMSAST* glms_ast_push_flag(GLMSAST* parent, GLMSAST* flag) {
    if (!parent || !flag) return 0;

  if (!parent->flags) {
    parent->flags = NEW(GLMSASTList);
  }

  if (!parent->flags->initialized) {
    glms_GLMSAST_list_init(parent->flags);
  }

  glms_GLMSAST_list_push(parent->flags, flag);

  return flag;
}


const char* glms_ast_get_name(GLMSAST* ast) {
  if (!ast) return 0;

  switch (ast->type) {
    case GLMS_AST_TYPE_BINOP: {
      return glms_ast_get_name(ast->as.binop.left);
    }; break;
    case GLMS_AST_TYPE_ID: {
      const char* value = glms_string_view_get_value(&ast->as.id.value);

      if (!value) {
        value = GLMS_TOKEN_TYPE_STR[ast->as.id.op];
      }

      return value;
    }; break;
    case GLMS_AST_TYPE_STRING: {
      return glms_string_view_get_value(&ast->as.string.value);
    }; break;
    case GLMS_AST_TYPE_FUNC: {
      if (!ast->as.func.id) return 0;
      return glms_string_view_get_value(&ast->as.func.id->as.id.value);
    }; break;
    case GLMS_AST_TYPE_TYPEDEF: {
      if (!ast->as.func.id) return 0;
      return glms_ast_get_name(ast->as.tdef.id);
    }; break;
    case GLMS_AST_TYPE_CALL: {
      if (!ast->as.call.left) return 0;
      return glms_ast_get_name(ast->as.call.left);
    }; break;
    default: { return 0; }; break;
  }
}

const char* glms_ast_get_string_value(GLMSAST* ast) {
  if (ast->type == GLMS_AST_TYPE_STRING && ast->as.string.heap != 0) {
    return ast->as.string.heap;
  }
  return glms_ast_get_name(ast);
}

const char* glms_ast_to_string_object(GLMSAST* ast) {
  if (ast->string_rep != 0) {
    free(ast->string_rep);
  }

  char* str = 0;

  text_append(&str, "{\n");

  if (ast->props.initialized) {
    HashyIterator it = {0};

    while (hashy_map_iterate(&ast->props, &it)) {
      if (!it.bucket->key) continue;
      if (!it.bucket->value) continue;

      const char* key = it.bucket->key;
      GLMSAST* value = (GLMSAST*)it.bucket->value;

      text_append(&str, "\"");
      text_append(&str, key);
      text_append(&str, "\"");
      text_append(&str, ": ");
      text_append(&str, glms_ast_to_string(value));
      text_append(&str, ",\n");
    }
  }

  text_append(&str, "}\n");

  ast->string_rep = str;

  return ast->string_rep;
}

const char* glms_ast_to_string_id(GLMSAST* ast) {
  if (ast->string_rep != 0) {
    free(ast->string_rep);
  }

  char* str = 0;

  text_append(&str, glms_ast_get_string_value(ast));

  if (ast->flags != 0) {
    for (int64_t i = 0; i < ast->flags->length; i++) {
      GLMSAST* flag = ast->flags->items[i];

      text_append(&str, "(");
      text_append(&str, glms_ast_to_string(flag));
      text_append(&str, ")");
    }
  }

  ast->string_rep = str;

  return ast->string_rep;
}

const char* glms_ast_to_string_typedef(GLMSAST* ast) {
  return glms_ast_to_string(ast->as.tdef.factor);
}

const char* glms_ast_to_string(GLMSAST* ast) {
  if (ast->type == GLMS_AST_TYPE_OBJECT || ast->type == GLMS_AST_TYPE_STRUCT) {
    return glms_ast_to_string_object(ast);
  } else if (ast->type == GLMS_AST_TYPE_ID) {
    return glms_ast_to_string_id(ast);
  }  else if (ast->type == GLMS_AST_TYPE_TYPEDEF) {
    return glms_ast_to_string_typedef(ast);
  }
  return GLMS_AST_TYPE_STR[ast->type];
}

bool glms_ast_is_truthy(GLMSAST* ast) {
  if (!ast) return false;

  switch (ast->type) {
    case GLMS_AST_TYPE_NUMBER: { return ast->as.number.value > 0; }; break;
    case GLMS_AST_TYPE_STRING: { return ast->as.string.value.length > 0 && ast->as.string.value.ptr != 0; }; break;
    case GLMS_AST_TYPE_UNDEFINED: { return false; }; break;
    case GLMS_AST_TYPE_EOF: { return false; }; break;
    case GLMS_AST_TYPE_NOOP: { return false; }; break;
    default: { return true; }; break;
  }

  return true;
}

GLMSAST* glms_ast_access_child_by_index(GLMSAST* ast, int64_t index) {
  if (!ast) return 0;
  if (ast->children == 0 || ast->children->length <= 0) return 0;

  if (index < 0 || index >= ast->children->length) GLMS_WARNING_RETURN(ast, stderr, "index out of bounds.\n");

  return ast->children->items[index];
}

GLMSAST* glms_ast_access_by_index(GLMSAST* ast, int64_t index, GLMSEnv* env) {
  if (!ast) return 0;

  switch (ast->type) {
    case GLMS_AST_TYPE_ARRAY: {
      return glms_ast_access_child_by_index(ast, index);
    }; break;
    case GLMS_AST_TYPE_STRING: {
      const char* string_value = glms_ast_get_string_value(ast);
      if (!string_value) GLMS_WARNING_RETURN(ast, stderr, "string == null.\n");
      int64_t len = strlen(string_value);
      if (index < 0 || index >= len) GLMS_WARNING_RETURN(ast, stderr, "index out of bounds.\n");
      char tmp[3];
      sprintf(tmp, "%c", string_value[index]);
      return glms_env_new_ast_string(env, tmp);
    }; break;
    default: {
      GLMS_WARNING_RETURN(ast, stderr, "value cannot be indexed.\n");
    }; break;
  }

  return ast;
}

GLMSAST* glms_ast_access_by_key(GLMSAST* ast, const char* key, GLMSEnv* env) {
  if (!ast || !key) return 0;

  if (ast->type == GLMS_AST_TYPE_UNDEFINED) GLMS_WARNING_RETURN(ast, stderr, "cannot index undefined.\n");
  if (ast->type == GLMS_AST_TYPE_NUMBER) GLMS_WARNING_RETURN(ast, stderr, "cannot index number.\n");
  if (!ast->props.initialized) return glms_env_new_ast(env, GLMS_AST_TYPE_UNDEFINED);
  GLMSAST* value = (GLMSAST*)hashy_map_get(&ast->props, key);
  if (!value) glms_env_new_ast(env, GLMS_AST_TYPE_UNDEFINED);


  return value;
}

bool glms_ast_compare_equals_equals(GLMSAST* a, GLMSAST* b) {
  if (a->type != b->type) return false;

  if (!a && !b) return true;
  if (!a || !b) return false;

  switch (a->type) {
    case GLMS_AST_TYPE_NUMBER: { return GLMSAST_VALUE(a) == GLMSAST_VALUE(b); }; break;
    case GLMS_AST_TYPE_STRING: {
      const char* str_a = glms_ast_get_string_value(a);
      const char* str_b = glms_ast_get_string_value(a);

      if (str_a == str_b) return true;
      if (!str_a || !str_b) return false;

      return strcmp(str_a, str_b) == 0;
    }; break;
    default: { return a == b; }; break;
  }

  return a == b;
}
bool glms_ast_compare_gt(GLMSAST* a, GLMSAST* b) {
  if (a->type != b->type) return false;

  if (a && !b) return true;
  if (!a && b) return false;
  if (!a || !b) return false;

  switch (a->type) {
    case GLMS_AST_TYPE_NUMBER: { return GLMSAST_VALUE(a) > GLMSAST_VALUE(b); }; break;
    default: {
      if (a->children && b->children) {
        return a->children->length > b->children->length;
      } else {
        return false;
      }
    }; break;
  }

  return false;
}
bool glms_ast_compare_gte(GLMSAST* a, GLMSAST* b) {
  if (a->type != b->type) return false;

  if (!a || !b) return false;

  switch (a->type) {
    case GLMS_AST_TYPE_NUMBER: { return GLMSAST_VALUE(a) >= GLMSAST_VALUE(b); }; break;
    default: {
      if (a->children && b->children) {
        return a->children->length >= b->children->length;
      } else {
        return false;
      }
    }; break;
  }

  return false;
}
bool glms_ast_compare_lt(GLMSAST* a, GLMSAST* b) {
  if (a->type != b->type) return false;

  if (!a && b) return true;
  if (a && !b) return false;
  if (!a || !b) return false;

  switch (a->type) {
    case GLMS_AST_TYPE_NUMBER: { return GLMSAST_VALUE(a) < GLMSAST_VALUE(b); }; break;
    default: {
      if (a->children && b->children) {
        return a->children->length < b->children->length;
      } else {
        return false;
      }
    }; break;
  }

  return false;
}
bool glms_ast_compare_lte(GLMSAST* a, GLMSAST* b) {
  if (a->type != b->type) return false;

  if (!a || !b) return false;

  switch (a->type) {
    case GLMS_AST_TYPE_NUMBER: { return GLMSAST_VALUE(a) <= GLMSAST_VALUE(b); }; break;
    default: {
      if (a->children && b->children) {
        return a->children->length <= b->children->length;
      } else {
        return false;
      }
    }; break;
  }

  return false;
}

GLMSAST* glms_ast_object_set_property(GLMSAST* obj, const char* key, GLMSAST* value) {
  if (!obj || !key) return 0;

  if (!obj->props.initialized) {
    hashy_map_init_v2(&obj->props, (HashyMapConfig){ .capacity = 256, .remember_keys = true });
  }

  if (value == 0) {
    hashy_map_unset(&obj->props, key);
  } else {
    hashy_map_set(&obj->props, key, value);
  }

  return obj;
}

float glms_ast_get_number_by_key(GLMSAST* ast, const char* key) {
  if (!ast || !key) return 0.0f;
  if (!ast->props.initialized) return 0.0f;

  GLMSAST* value = (GLMSAST*)hashy_map_get(&ast->props, key);

  if (!value) return 0.0f;
  if (value->type != GLMS_AST_TYPE_NUMBER) return 0.0f;

  return GLMSAST_VALUE(value);
}

GLMSAST* glms_ast_get_type(GLMSAST* ast) {
  if (!ast) return 0;
  if (!ast->flags) return 0;
  if (!ast->flags->length) return 0;

  for (int64_t i = 0; i < ast->flags->length; i++) {
    GLMSAST* flag = ast->flags->items[i];
    if (flag->type != GLMS_AST_TYPE_ID) GLMS_WARNING_RETURN(0, stderr, "Invalid non-ID flag.\n");

    GLMSTokenType t = flag->as.id.op;

    if (
      t != GLMS_TOKEN_TYPE_ID &&
      (t != GLMS_TOKEN_TYPE_SPECIAL_LET && t != GLMS_TOKEN_TYPE_SPECIAL_CONST)
    ) {
      return flag;
    }
  }

  return 0;
}

GLMSAST* glms_ast_copy(GLMSAST src, GLMSEnv* env) {
  if (!env) return 0;


  GLMSAST* dest = glms_env_new_ast(env, src.type);
  *dest = src;
  dest->props = (HashyMap){0};
  dest->children = 0;
  dest->flags = 0;
  dest->string_rep = 0;

  if (src.children != 0) {
    for (int64_t i = 0; i < src.children->length; i++) {
      GLMSAST* child = src.children->items[i];
      GLMSAST* copied = glms_ast_copy(*child, env);
      glms_ast_push(dest, copied);
    }
  }

  if (src.flags != 0) {
    for (int64_t i = 0; i < src.flags->length; i++) {
      GLMSAST* child = src.flags->items[i];
      GLMSAST* copied = glms_ast_copy(*child, env);
      glms_ast_push_flag(dest, copied);
    }
  }


  if (src.props.initialized) {
    hashy_map_init_v2(&dest->props, src.props.config);
    HashyIterator it = {0};


    while (hashy_map_iterate(&src.props, &it)) {
      if (!it.bucket->key) continue;
      if (!it.bucket->value) continue;

      const char* key = it.bucket->key;
      GLMSAST* value = (GLMSAST*)it.bucket->value;

      GLMSAST* copied = glms_ast_copy(*value, env);

      if (copied == 0) {
        GLMS_WARNING(stderr, "Failed to copy.\n");
        continue;
      }
      glms_ast_object_set_property(dest, key, copied);
    }
  }

  return dest;
}
