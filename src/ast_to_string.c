#include "glms/ast_type.h"
#include "glms/string_view.h"
#include <glms/ast.h>
#include <glms/macros.h>
#include <string.h>

char *glms_ast_to_string(GLMSAST ast, GLMSAllocator alloc) {
  if (!alloc.strdup)
    GLMS_WARNING_RETURN(0, stderr, "No allocator provided.\n");
  if (ast.to_string) {
    return ast.to_string(&ast, alloc);
  }

  switch (ast.type) {
    case GLMS_AST_TYPE_STACK_PTR: {
	GLMSAST *ptr = 0;
	if ((ptr = glms_ast_get_ptr(ast))) {
	return glms_ast_to_string(*ptr, alloc);
	}
    }; break;
    case GLMS_AST_TYPE_STRING: {
	const char *v = glms_string_view_get_value(&ast.as.string.value);
	if (v != 0) {
	return alloc.strdup(alloc.user_ptr, v);
	}
    }; break;
    case GLMS_AST_TYPE_NUMBER: {
	char tmp[256];
	sprintf(tmp, "%1.6f", ast.as.number.value);
	return alloc.strdup(alloc.user_ptr, tmp);
    }; break;
    default: {
	return alloc.strdup(alloc.user_ptr, GLMS_AST_TYPE_STR[ast.type]);
    }; break;
  }

  return 0;
}
