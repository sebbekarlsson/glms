#include <jscript/ast.h>
#include <jscript/macros.h>
#include <jscript/env.h>


JSCRIPT_IMPLEMENT_BUFFER(JSCRIPTAST);
JSCRIPT_IMPLEMENT_LIST(JSCRIPTAST);

bool jscript_ast_is_iterable(JSCRIPTAST* ast) {
  if (!ast) return false;
  return ast->type == JSCRIPT_AST_TYPE_COMPOUND;
}

JSCRIPTAST* jscript_ast_push(JSCRIPTAST* parent, JSCRIPTAST* child) {
  if (!parent || !child) return 0;

  if (!parent->children) {
    parent->children = NEW(JSCRIPTASTList);
  }

  if (!parent->children->initialized) {
    jscript_JSCRIPTAST_list_init(parent->children);
  }

  jscript_JSCRIPTAST_list_push(parent->children, child);

  return child;
}

const char* jscript_ast_get_name(JSCRIPTAST* ast) {
  if (!ast) return 0;

  switch (ast->type) {
    case JSCRIPT_AST_TYPE_BINOP: {
      return jscript_ast_get_name(ast->as.binop.left);
    }; break;
    case JSCRIPT_AST_TYPE_ID: {
      return jscript_string_view_get_value(&ast->as.id.value);
    }; break;
    case JSCRIPT_AST_TYPE_STRING: {
      return jscript_string_view_get_value(&ast->as.string.value);
    }; break;
    case JSCRIPT_AST_TYPE_FUNC: {
      if (!ast->as.func.id) return 0;
      return jscript_string_view_get_value(&ast->as.func.id->as.id.value);
    }; break;
    default: { return 0; }; break;
  }
}

const char* jscript_ast_get_string_value(JSCRIPTAST* ast) {
  if (ast->type == JSCRIPT_AST_TYPE_STRING && ast->as.string.heap != 0) {
    return ast->as.string.heap;
  }
  return jscript_ast_get_name(ast);
}

const char* jscript_ast_to_string(JSCRIPTAST* ast) {
  return JSCRIPT_AST_TYPE_STR[ast->type];
}

bool jscript_ast_is_truthy(JSCRIPTAST* ast) {
  if (!ast) return false;

  switch (ast->type) {
    case JSCRIPT_AST_TYPE_NUMBER: { return ast->as.number.value > 0; }; break;
    case JSCRIPT_AST_TYPE_STRING: { return ast->as.string.value.length > 0 && ast->as.string.value.ptr != 0; }; break;
    default: { return true; }; break;
  }

  return true;
}

JSCRIPTAST* jscript_ast_access_child_by_index(JSCRIPTAST* ast, int64_t index) {
  if (!ast) return 0;
  if (ast->children == 0 || ast->children->length <= 0) return 0;

  if (index < 0 || index >= ast->children->length) JSCRIPT_WARNING_RETURN(ast, stderr, "index out of bounds.\n");

  return ast->children->items[index];
}

JSCRIPTAST* jscript_ast_access_by_index(JSCRIPTAST* ast, int64_t index, JSCRIPTEnv* env) {
  if (!ast) return 0;

  switch (ast->type) {
    case JSCRIPT_AST_TYPE_ARRAY: {
      return jscript_ast_access_child_by_index(ast, index);
    }; break;
    case JSCRIPT_AST_TYPE_STRING: {
      const char* string_value = jscript_ast_get_string_value(ast);
      if (!string_value) JSCRIPT_WARNING_RETURN(ast, stderr, "string == null.\n");
      int64_t len = strlen(string_value);
      if (index < 0 || index >= len) JSCRIPT_WARNING_RETURN(ast, stderr, "index out of bounds.\n");
      char tmp[3];
      sprintf(tmp, "%c", string_value[index]);
      return jscript_env_new_ast_string(env, tmp);
    }; break;
    default: {
      JSCRIPT_WARNING_RETURN(ast, stderr, "value cannot be indexed.\n");
    }; break;
  }

  return ast;
}

bool jscript_ast_compare_equals_equals(JSCRIPTAST* a, JSCRIPTAST* b) {
  if (a->type != b->type) return false;

  if (!a && !b) return true;
  if (!a || !b) return false;

  switch (a->type) {
    case JSCRIPT_AST_TYPE_NUMBER: { return JSCRIPTAST_VALUE(a) == JSCRIPTAST_VALUE(b); }; break;
    case JSCRIPT_AST_TYPE_STRING: {
      const char* str_a = jscript_ast_get_string_value(a);
      const char* str_b = jscript_ast_get_string_value(a);

      if (str_a == str_b) return true;
      if (!str_a || !str_b) return false;

      return strcmp(str_a, str_b) == 0;
    }; break;
    default: { return a == b; }; break;
  }

  return a == b;
}
bool jscript_ast_compare_gt(JSCRIPTAST* a, JSCRIPTAST* b) {
  if (a->type != b->type) return false;

  if (a && !b) return true;
  if (!a && b) return false;
  if (!a || !b) return false;

  switch (a->type) {
    case JSCRIPT_AST_TYPE_NUMBER: { return JSCRIPTAST_VALUE(a) > JSCRIPTAST_VALUE(b); }; break;
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
bool jscript_ast_compare_gte(JSCRIPTAST* a, JSCRIPTAST* b) {
  if (a->type != b->type) return false;

  if (!a || !b) return false;

  switch (a->type) {
    case JSCRIPT_AST_TYPE_NUMBER: { return JSCRIPTAST_VALUE(a) >= JSCRIPTAST_VALUE(b); }; break;
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
bool jscript_ast_compare_lt(JSCRIPTAST* a, JSCRIPTAST* b) {
  if (a->type != b->type) return false;

  if (!a && b) return true;
  if (a && !b) return false;
  if (!a || !b) return false;

  switch (a->type) {
    case JSCRIPT_AST_TYPE_NUMBER: { return JSCRIPTAST_VALUE(a) < JSCRIPTAST_VALUE(b); }; break;
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
bool jscript_ast_compare_lte(JSCRIPTAST* a, JSCRIPTAST* b) {
  if (a->type != b->type) return false;

  if (!a || !b) return false;

  switch (a->type) {
    case JSCRIPT_AST_TYPE_NUMBER: { return JSCRIPTAST_VALUE(a) <= JSCRIPTAST_VALUE(b); }; break;
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
