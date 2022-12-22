#include <jscript/ast.h>
#include <jscript/macros.h>


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
