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
