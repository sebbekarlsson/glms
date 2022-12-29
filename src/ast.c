#include "hashy/hashy.h"
#include <glms/ast.h>
#include <glms/env.h>
#include <glms/macros.h>
#include <text/text.h>

GLMS_IMPLEMENT_BUFFER(GLMSAST);
GLMS_IMPLEMENT_LIST(GLMSAST);

bool glms_ast_is_iterable(GLMSAST *ast) {
  if (!ast)
    return false;
  return ast->type == GLMS_AST_TYPE_COMPOUND;
}

GLMSAST *glms_ast_push(GLMSAST *parent, GLMSAST *child) {
  if (!parent || !child)
    return 0;

  if (!parent->children) {
    parent->children = NEW(GLMSASTList);
  }

  if (!parent->children->initialized) {
    glms_GLMSAST_list_init(parent->children);
  }

  glms_GLMSAST_list_push(parent->children, child);

  return child;
}

GLMSAST *glms_ast_push_flag(GLMSAST *parent, GLMSAST *flag) {
  if (!parent || !flag)
    return 0;

  if (!parent->flags) {
    parent->flags = NEW(GLMSASTList);
  }

  if (!parent->flags->initialized) {
    glms_GLMSAST_list_init(parent->flags);
  }

  glms_GLMSAST_list_push(parent->flags, flag);

  return flag;
}

const char *glms_ast_get_name(GLMSAST *ast) {
  if (!ast)
    return 0;

  switch (ast->type) {
  case GLMS_AST_TYPE_BINOP: {
    return glms_ast_get_name(ast->as.binop.left);
  }; break;
  case GLMS_AST_TYPE_ID: {

    if (ast->as.id.heap != 0) {
      return ast->as.id.heap;
    }

    const char *value = glms_string_view_get_value(&ast->as.id.value);

    if (!value) {
      value = GLMS_TOKEN_TYPE_STR[ast->as.id.op];
    }

    return value;
  }; break;
  case GLMS_AST_TYPE_STRING: {
    return glms_string_view_get_value(&ast->as.string.value);
  }; break;
  case GLMS_AST_TYPE_FUNC: {
    if (!ast->as.func.id)
      return 0;
    return glms_string_view_get_value(&ast->as.func.id->as.id.value);
  }; break;
  case GLMS_AST_TYPE_TYPEDEF: {
    if (!ast->as.func.id)
      return 0;
    return glms_ast_get_name(ast->as.tdef.id);
  }; break;
  case GLMS_AST_TYPE_CALL: {
    if (!ast->as.call.left)
      return 0;
    return glms_ast_get_name(ast->as.call.left);
  }; break;
  default: {
    return 0;
  }; break;
  }
}

const char *glms_ast_get_string_value(GLMSAST *ast) {
  if (ast->type == GLMS_AST_TYPE_STRING && ast->as.string.heap != 0) {
    return ast->as.string.heap;
  }
  return glms_ast_get_name(ast);
}

const char *glms_ast_to_string_array(GLMSAST *ast) {

  if (ast->string_rep != 0) {
    free(ast->string_rep);
    ast->string_rep = 0;
  }

  char* str = 0;

  text_append(&str, "[");

  if (ast->children != 0) {
    for (int64_t i = 0; i < ast->children->length; i++) {
      GLMSAST* child = ast->children->items[i];
      const char* childstr = glms_ast_to_string(child);
      if (!childstr) continue;

      text_append(&str, childstr);

      if (i < ast->children->length-1) {
	text_append(&str, ", ");
      }
    }
  }
  
  text_append(&str, "]");

  ast->string_rep = str;

  return ast->string_rep;
}

const char *glms_ast_to_string_object(GLMSAST *ast) {
  if (ast->string_rep != 0) {
    free(ast->string_rep);
  }

  char *str = 0;

  text_append(&str, "{\n");

  if (ast->props.initialized) {
    HashyIterator it = {0};

    while (hashy_map_iterate(&ast->props, &it)) {
      if (!it.bucket->key)
        continue;
      if (!it.bucket->value)
        continue;

      const char *key = it.bucket->key;
      GLMSAST *value = (GLMSAST *)it.bucket->value;

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

const char *glms_ast_to_string_id(GLMSAST *ast) {
  if (ast->string_rep != 0) {
    free(ast->string_rep);
  }

  char *str = 0;

  text_append(&str, glms_ast_get_string_value(ast));

  if (ast->flags != 0) {
    for (int64_t i = 0; i < ast->flags->length; i++) {
      GLMSAST *flag = ast->flags->items[i];

      text_append(&str, "(");
      text_append(&str, glms_ast_to_string(flag));
      text_append(&str, ")");
    }
  }

  ast->string_rep = str;

  return ast->string_rep;
}

const char *glms_ast_to_string_typedef(GLMSAST *ast) {
  return glms_ast_to_string(ast->as.tdef.factor);
}

const char *glms_ast_to_string_number(GLMSAST *ast) {
  char tmp[256];
  sprintf(tmp, "%1.6f", ast->as.number.value);

  if (ast->string_rep) {
    free(ast->string_rep);
    ast->string_rep = 0;
  }

  ast->string_rep = strdup(tmp);
  return ast->string_rep;
}

const char *glms_ast_to_string_bool(GLMSAST *ast) {
  char tmp[256];
  sprintf(tmp, "%s", ast->as.boolean ? "true" : "false");

  if (ast->string_rep) {
    free(ast->string_rep);
    ast->string_rep = 0;
  }

  ast->string_rep = strdup(tmp);
  return ast->string_rep;
}

const char *glms_ast_to_string(GLMSAST *ast) {
  if (ast->to_string != 0) {
    return ast->to_string(ast);
  }
  if (ast->type == GLMS_AST_TYPE_OBJECT || ast->type == GLMS_AST_TYPE_STRUCT) {
    return glms_ast_to_string_object(ast);
  } else if (ast->type == GLMS_AST_TYPE_ID) {
    return glms_ast_to_string_id(ast);
  } else if (ast->type == GLMS_AST_TYPE_TYPEDEF) {
    return glms_ast_to_string_typedef(ast);
  } else if (ast->type == GLMS_AST_TYPE_NUMBER) {
    return glms_ast_to_string_number(ast);
  } else if (ast->type == GLMS_AST_TYPE_STRING) {
    return glms_ast_get_string_value(ast);
  } else if (ast->type == GLMS_AST_TYPE_ARRAY) {
    return glms_ast_to_string_array(ast);
  } else if (ast->type == GLMS_AST_TYPE_BOOL) {
    return glms_ast_to_string_bool(ast);
  }
  return GLMS_AST_TYPE_STR[ast->type];
}

bool glms_ast_is_truthy(GLMSAST *ast) {
  if (!ast)
    return false;

  switch (ast->type) {
  case GLMS_AST_TYPE_BOOL: {
    return ast->as.boolean;
  }; break;
  case GLMS_AST_TYPE_NUMBER: {
    return ast->as.number.value > 0;
  }; break;
  case GLMS_AST_TYPE_STRING: {
    return ast->as.string.value.length > 0 && ast->as.string.value.ptr != 0;
  }; break;
  case GLMS_AST_TYPE_UNDEFINED: {
    return false;
  }; break;
  case GLMS_AST_TYPE_EOF: {
    return false;
  }; break;
  case GLMS_AST_TYPE_NOOP: {
    return false;
  }; break;
  default: {
    return true;
  }; break;
  }

  return true;
}

GLMSAST *glms_ast_access_child_by_index(GLMSAST *ast, int64_t index) {
  if (!ast)
    return 0;
  if (ast->children == 0 || ast->children->length <= 0)
    return 0;

  if (index < 0 || index >= ast->children->length)
    GLMS_WARNING_RETURN(ast, stderr, "index out of bounds.\n");

  return ast->children->items[index];
}

GLMSAST *glms_ast_access_by_index(GLMSAST *ast, int64_t index, GLMSEnv *env) {
  if (!ast)
    return 0;

  switch (ast->type) {
  case GLMS_AST_TYPE_ARRAY: {
    return glms_ast_access_child_by_index(ast, index);
  }; break;
  case GLMS_AST_TYPE_STRING: {
    const char *string_value = glms_ast_get_string_value(ast);
    if (!string_value)
      GLMS_WARNING_RETURN(ast, stderr, "string == null.\n");
    int64_t len = strlen(string_value);
    if (index < 0 || index >= len)
      GLMS_WARNING_RETURN(ast, stderr, "index out of bounds.\n");
    char tmp[3];
    sprintf(tmp, "%c", string_value[index]);
    return glms_env_new_ast_string(env, tmp, true);
  }; break;
  default: {
    GLMS_WARNING_RETURN(ast, stderr, "value cannot be indexed.\n");
  }; break;
  }

  return ast;
}

GLMSAST *glms_ast_access_by_key(GLMSAST *ast, const char *key, GLMSEnv *env) {
  if (!ast || !key)
    return 0;

  if (ast->type == GLMS_AST_TYPE_UNDEFINED)
    GLMS_WARNING_RETURN(ast, stderr, "cannot index undefined.\n");
  if (ast->type == GLMS_AST_TYPE_NUMBER)
    GLMS_WARNING_RETURN(ast, stderr, "cannot index number.\n");
  if (!ast->props.initialized)
    return env->undefined; 
  GLMSAST *value = (GLMSAST *)hashy_map_get(&ast->props, key);
  if (!value) return env->undefined;

  return value;
}

bool glms_ast_compare_equals_equals(GLMSAST *a, GLMSAST *b) {
  if (a->type != b->type)
    return false;

  if (!a && !b)
    return true;
  if (!a || !b)
    return false;

  switch (a->type) {
  case GLMS_AST_TYPE_NUMBER: {
    return GLMSAST_VALUE(a) == GLMSAST_VALUE(b);
  }; break;
  case GLMS_AST_TYPE_STRING: {
    const char *str_a = glms_ast_get_string_value(a);
    const char *str_b = glms_ast_get_string_value(a);

    if (str_a == str_b)
      return true;
    if (!str_a || !str_b)
      return false;

    return strcmp(str_a, str_b) == 0;
  }; break;
  default: {
    return a == b;
  }; break;
  }

  return a == b;
}
bool glms_ast_compare_gt(GLMSAST *a, GLMSAST *b) {
  if (a->type != b->type)
    return false;

  if (a && !b)
    return true;
  if (!a && b)
    return false;
  if (!a || !b)
    return false;

  switch (a->type) {
  case GLMS_AST_TYPE_NUMBER: {
    return GLMSAST_VALUE(a) > GLMSAST_VALUE(b);
  }; break;
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
bool glms_ast_compare_gte(GLMSAST *a, GLMSAST *b) {
  if (a->type != b->type)
    return false;

  if (!a || !b)
    return false;

  switch (a->type) {
  case GLMS_AST_TYPE_NUMBER: {
    return GLMSAST_VALUE(a) >= GLMSAST_VALUE(b);
  }; break;
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
bool glms_ast_compare_lt(GLMSAST *a, GLMSAST *b) {
  if (a->type != b->type)
    return false;

  if (!a && b)
    return true;
  if (a && !b)
    return false;
  if (!a || !b)
    return false;

  switch (a->type) {
  case GLMS_AST_TYPE_NUMBER: {
    return GLMSAST_VALUE(a) < GLMSAST_VALUE(b);
  }; break;
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
bool glms_ast_compare_lte(GLMSAST *a, GLMSAST *b) {
  if (a->type != b->type)
    return false;

  if (!a || !b)
    return false;

  switch (a->type) {
  case GLMS_AST_TYPE_NUMBER: {
    return GLMSAST_VALUE(a) <= GLMSAST_VALUE(b);
  }; break;
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

GLMSAST *glms_ast_object_set_property(GLMSAST *obj, const char *key,
                                      GLMSAST *value) {
  if (!obj || !key)
    return 0;

  if (!obj->props.initialized) {
    hashy_map_init_v2(&obj->props,
                      (HashyMapConfig){.capacity = 256, .remember_keys = true});
  }

  if (value == 0) {
    hashy_map_unset(&obj->props, key);
  } else {
    hashy_map_set(&obj->props, key, value);
  }

  return obj;
}

float glms_ast_get_number_by_key(GLMSAST *ast, const char *key) {
  if (!ast || !key)
    return 0.0f;
  if (!ast->props.initialized)
    return 0.0f;

  GLMSAST *value = (GLMSAST *)hashy_map_get(&ast->props, key);

  if (!value)
    return 0.0f;
  if (value->type != GLMS_AST_TYPE_NUMBER)
    return 0.0f;

  return GLMSAST_VALUE(value);
}

GLMSAST *glms_ast_get_type(GLMSAST *ast) {
  if (!ast)
    return 0;
  if (!ast->flags)
    return 0;
  if (!ast->flags->length)
    return 0;

  if (ast->value_type) return ast->value_type;

  if (ast->type == GLMS_AST_TYPE_BINOP) {
    GLMSAST* flag = glms_ast_get_type(ast->as.binop.left);

    if (flag != 0) return flag;

    flag = glms_ast_get_type(ast->as.binop.right);

    if (flag != 0) return flag;
  }

  for (int64_t i = 0; i < ast->flags->length; i++) {
    GLMSAST *flag = ast->flags->items[i];
    if (flag->type != GLMS_AST_TYPE_ID)
      GLMS_WARNING_RETURN(0, stderr, "Invalid non-ID flag.\n");

    GLMSTokenType t = flag->as.id.op;

    if ((t != GLMS_TOKEN_TYPE_SPECIAL_LET &&
                                    t != GLMS_TOKEN_TYPE_SPECIAL_CONST)) {
      return flag;
    }
  }

  return 0;
}

GLMSAST *glms_ast_copy(GLMSAST src, GLMSEnv *env) {
  if (!env)
    return 0;

  GLMSAST *dest = glms_env_new_ast(env, src.type, true);
  *dest = src;
  dest->props = (HashyMap){0};
  dest->children = 0;
  dest->flags = 0;
  dest->string_rep = 0;
  dest->ptr =  src.ptr;

  if (src.children != 0) {
    for (int64_t i = 0; i < src.children->length; i++) {
      GLMSAST *child = src.children->items[i];
      GLMSAST *copied = glms_ast_copy(*child, env);
      glms_ast_push(dest, copied);
    }
  }

  if (src.flags != 0) {
    for (int64_t i = 0; i < src.flags->length; i++) {
      GLMSAST *child = src.flags->items[i];
      GLMSAST *copied = glms_ast_copy(*child, env);
      glms_ast_push_flag(dest, copied);
    }
  }

  if (src.props.initialized) {
    hashy_map_init_v2(&dest->props, src.props.config);
    HashyIterator it = {0};

    while (hashy_map_iterate(&src.props, &it)) {
      if (!it.bucket->key)
        continue;
      if (!it.bucket->value)
        continue;

      const char *key = it.bucket->key;
      GLMSAST *value = (GLMSAST *)it.bucket->value;

      GLMSAST *copied = glms_ast_copy(*value, env);

      if (copied == 0) {
        GLMS_WARNING(stderr, "Failed to copy.\n");
        continue;
      }
      glms_ast_object_set_property(dest, key, copied);
    }
  }

  return dest;
}

void glms_ast_destructor_binop(GLMSAST *ast) {
  if (ast->as.binop.left != 0) {
    glms_ast_destructor(ast->as.binop.left);
    ast->as.binop.left = 0;
  }

  if (ast->as.binop.right != 0) {
    glms_ast_destructor(ast->as.binop.right);
    ast->as.binop.right = 0;
  }
}

void glms_ast_destructor_unop(GLMSAST *ast) {
  if (ast->as.unop.left != 0) {
    glms_ast_destructor(ast->as.unop.left);
    ast->as.unop.left = 0;
  }

  if (ast->as.unop.right != 0) {
    glms_ast_destructor(ast->as.unop.right);
    ast->as.unop.right = 0;
  }
}

void glms_ast_destructor_access(GLMSAST *ast) {
  if (ast->as.access.left != 0) {
    glms_ast_destructor(ast->as.access.left);
    ast->as.access.left = 0;
  }

  if (ast->as.access.right != 0) {
    glms_ast_destructor(ast->as.access.right);
    ast->as.access.right = 0;
  }
}
void glms_ast_destructor_call(GLMSAST *ast) {
  if (ast->as.call.left != 0) {
    glms_ast_destructor(ast->as.call.left);
    ast->as.call.left = 0;
  }

  if (ast->as.call.right != 0) {
    glms_ast_destructor(ast->as.call.right);
    ast->as.call.right = 0;
  }
}
void glms_ast_destructor_string(GLMSAST *ast) {
  if (ast->as.string.heap != 0) {
    free(ast->as.string.heap);
    ast->as.string.heap = 0;
  }
}
void glms_ast_destructor_id(GLMSAST *ast) {
  if (ast->as.id.heap != 0) {
    // TODO: this is a double free somehow, for some reason. Fix it!
    // free(ast->as.id.heap);
    ast->as.id.heap = 0;
  }
}

void glms_ast_destructor_func(GLMSAST *ast) {
  if (ast->as.func.body != 0) {
    glms_ast_destructor(ast->as.func.body);
    ast->as.func.body = 0;
  }

  if (ast->as.func.id != 0) {
    glms_ast_destructor(ast->as.func.id);
    ast->as.func.id = 0;
  }
}

void glms_ast_destructor_typedef(GLMSAST *ast) {
  if (ast->as.tdef.id != 0) {
    glms_ast_destructor(ast->as.tdef.id);
    ast->as.tdef.id = 0;
  }

  if (ast->as.tdef.factor != 0) {
    glms_ast_destructor(ast->as.tdef.factor);
    ast->as.tdef.factor = 0;
  }
}

void glms_ast_destructor_block(GLMSAST *ast) {
  if (ast->as.block.body != 0) {
    glms_ast_destructor(ast->as.block.body);
    ast->as.block.body = 0;
  }

  if (ast->as.block.expr != 0) {
    glms_ast_destructor(ast->as.block.expr);
    ast->as.block.expr = 0;
  }

  if (ast->as.block.next != 0) {
    glms_ast_destructor(ast->as.block.next);
    ast->as.block.next = 0;
  }
}

void glms_ast_destructor(GLMSAST *ast) {
  if (!ast)
    return;

  if (ast->destructor) {
    ast->destructor(ast);
  }
  
  switch (ast->type) {
  case GLMS_AST_TYPE_BINOP: {
    glms_ast_destructor_binop(ast);
  }; break;
  case GLMS_AST_TYPE_UNOP: {
    glms_ast_destructor_unop(ast);
  }; break;
  case GLMS_AST_TYPE_ACCESS: {
    glms_ast_destructor_access(ast);
  }; break;
  case GLMS_AST_TYPE_STRING: {
    glms_ast_destructor_string(ast);
  }; break;
  case GLMS_AST_TYPE_ID: {
    glms_ast_destructor_id(ast);
  }; break;
  case GLMS_AST_TYPE_CALL: {
    glms_ast_destructor_call(ast);
  }; break;
  case GLMS_AST_TYPE_FUNC: {
    glms_ast_destructor_func(ast);
  }; break;
  case GLMS_AST_TYPE_TYPEDEF: {
    glms_ast_destructor_typedef(ast);
  }; break;
  case GLMS_AST_TYPE_BLOCK: {
    glms_ast_destructor_block(ast);
  }; break;
  default: {
  }; break;
  }

  if (ast->string_rep != 0) {
    free(ast->string_rep);
  }
  ast->string_rep = 0;
  if (ast->children != 0) {

    for (int64_t i = 0; i < ast->children->length; i++) {
      glms_ast_destructor(ast->children->items[i]);
    }

    glms_GLMSAST_list_clear(ast->children);
    free(ast->children);
  }
  ast->children = 0;

  if (ast->flags != 0) {

    for (int64_t i = 0; i < ast->flags->length; i++) {
      glms_ast_destructor(ast->flags->items[i]);
    }

    glms_GLMSAST_list_clear(ast->flags);
    free(ast->flags);
  }
  ast->flags = 0;

  ast->fptr = 0;
  hashy_map_clear(&ast->props, false);
}

int64_t glms_ast_array_get_length(GLMSAST *ast) {
  if (!ast)  return 0;
  if (!ast->children) return 0;
  return ast->children->length;
}

bool glms_ast_is_vector(GLMSAST *ast) {
  if (!ast) return false;
  return (ast->type == GLMS_AST_TYPE_VEC3 || ast->type == GLMS_AST_TYPE_VEC2 || ast->type == GLMS_AST_TYPE_VEC4);
}

void glms_ast_keep(GLMSAST *ast) {
  if (!ast) return;
  ast->keep = true;
}

GLMSAST *glms_ast_get_property(GLMSAST *ast, const char *key) {
  if (!ast || !key) return 0;
  if (!ast->props.initialized) return 0;

  return (GLMSAST*)hashy_map_get(&ast->props, key);
  
}
