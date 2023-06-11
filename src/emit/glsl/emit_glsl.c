#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/eval.h"
#include "glms/macros.h"
#include "glms/string_builder.h"
#include "glms/token.h"
#include <endian.h>
#include <glms/emit/glsl/emit_glsl.h>
#include <string.h>
#include <glms/env.h>

#define INDENT_NUM 2

static int glms_emit_glsl_(GLMSEmit *emit, GLMSAST *ast, int indent);


#define EMIT_APPEND(v) glms_string_builder_append(&emit->builder, v)
#define EMIT_APPEND_INDENTED(v, indent) glms_string_builder_append_indented(&emit->builder, v, indent, " ")

#define EMIT_APPEND_TYPE(typ)\
  {\
    const char* to_str = GLMS_AST_TYPE_STR[typ];\
    char tmp[512];\
    memset(&tmp[0], 0, 512*sizeof(char));\
    sprintf(tmp, "/* %s */", to_str);\
    EMIT_APPEND(tmp);\
  }


#define EMIT_TOKEN_TYPE(op, indent)                    \
   switch (op) {\
   case GLMS_TOKEN_TYPE_LT: EMIT_APPEND(" < "); break;       \
   case GLMS_TOKEN_TYPE_LTE: EMIT_APPEND(" <= "); break;       \
   case GLMS_TOKEN_TYPE_GT: EMIT_APPEND(" > "); break;       \
   case GLMS_TOKEN_TYPE_GTE: EMIT_APPEND(" >= "); break;       \
   case GLMS_TOKEN_TYPE_EQUALS: EMIT_APPEND(" = "); break;       \
   case GLMS_TOKEN_TYPE_EQUALS_EQUALS: EMIT_APPEND(" == "); break;       \
   case GLMS_TOKEN_TYPE_EXCLAM: EMIT_APPEND("!"); break;       \
   case GLMS_TOKEN_TYPE_ADD: EMIT_APPEND(" + "); break;       \
   case GLMS_TOKEN_TYPE_ADD_EQUALS: EMIT_APPEND(" += "); break; \
   case GLMS_TOKEN_TYPE_ADD_ADD: EMIT_APPEND(" ++ "); break;  \
   case GLMS_TOKEN_TYPE_SUB: EMIT_APPEND(" - "); break;       \
   case GLMS_TOKEN_TYPE_SUB_EQUALS: EMIT_APPEND(" -= "); break; \
   case GLMS_TOKEN_TYPE_SUB_SUB: EMIT_APPEND(" -- "); break;  \
   case GLMS_TOKEN_TYPE_MUL: EMIT_APPEND(" * "); break;       \
   case GLMS_TOKEN_TYPE_MUL_EQUALS: EMIT_APPEND(" *= "); break; \
   case GLMS_TOKEN_TYPE_DIV: EMIT_APPEND(" / "); break;       \
   case GLMS_TOKEN_TYPE_DIV_EQUALS: EMIT_APPEND(" /= "); break; \
   case GLMS_TOKEN_TYPE_DOT: EMIT_APPEND("."); break;         \
   case GLMS_TOKEN_TYPE_SPECIAL_RETURN: { EMIT_APPEND_INDENTED("return ", indent); }; break; \
   default: EMIT_APPEND(" ? "); break;                        \
  }

static int glms_emit_glsl_as_is(GLMSEmit *emit, GLMSAST *ast) {

  EMIT_APPEND_TYPE(ast->type);

  return 1;
  /* 
  const char *value = glms_ast_get_string_value(ast);
  if (!value)
    return 0;
  glms_string_builder_append(&emit->builder, value);
  return 1;*/
}

static int glms_emit_glsl_stack_ptr(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_func_overload_ptr(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_string(GLMSEmit *emit, GLMSAST *ast, int indent) {
  const char* value = glms_ast_get_string_value(ast);
  if (!value) return 0;
  EMIT_APPEND_INDENTED(value, indent);
  return 1;
}
static int glms_emit_glsl_raw_glsl(GLMSEmit *emit, GLMSAST *ast, int indent) {
  if (ast->as.raw_glsl.right == 0) return 0;
  return glms_emit_glsl_(emit, ast->as.raw_glsl.right, indent);
}
static int glms_emit_glsl_char(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_number(GLMSEmit *emit, GLMSAST *ast, int indent) {
  float value = glms_ast_number(*ast);

  char tmp[32];
  memset(&tmp[0], 0, 32*sizeof(char));

  switch (ast->as.number.type) {
  case GLMS_AST_NUMBER_TYPE_INT: {
    sprintf(&tmp[0], "%d", (int)value);
    EMIT_APPEND_INDENTED(tmp, indent);
  }; break;

  default:
  case GLMS_AST_NUMBER_TYPE_FLOAT: {
    sprintf(&tmp[0], "%1.6f", value);
    EMIT_APPEND_INDENTED(tmp, indent);
  }; break;
  }

  return 1;
}
static int glms_emit_glsl_bool(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_array(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_vec2(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_vec3(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_vec4(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_mat3(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_mat4(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_typedef(GLMSEmit *emit, GLMSAST *ast, int indent) {
  GLMSAST* id = ast->as.tdef.id;
  GLMSAST* factor = ast->as.tdef.factor;

  EMIT_APPEND_INDENTED("typedef ", indent);
  
  if (factor != 0) {
    glms_emit_glsl_(emit, factor, indent);
  }
  
  if (id != 0) {
    glms_emit_glsl_(emit, id, indent);
  }


  return 1;
}
static int glms_emit_glsl_object(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_struct(GLMSEmit *emit, GLMSAST *ast, int indent) {
  EMIT_APPEND_INDENTED("struct {\n", indent);
  if (ast->props.initialized == true) {
    for (int64_t i = 0; i < ast->props.keys.length; i++) {
      const char* key = ast->props.keys.items[i];
      if (!key) continue;
      GLMSAST* child = glms_ast_get_property(ast, key);
      if (!child) continue;
      glms_emit_glsl_(emit, child, indent + INDENT_NUM);
      EMIT_APPEND_INDENTED(";\n", indent);
    }
  }
  EMIT_APPEND_INDENTED("\n}", indent);
  return 1;
}
static int glms_emit_glsl_enum(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_binop(GLMSEmit *emit, GLMSAST *ast, int indent) {
  GLMSAST* left = ast->as.binop.left;
  GLMSAST* right = ast->as.binop.right;
  GLMSTokenType op = ast->as.binop.op;

  if (left != 0) {
    glms_emit_glsl_(emit, left, indent);
  }

  EMIT_TOKEN_TYPE(op, indent);
  
  if (right != 0) {
    glms_emit_glsl_(emit, right, 0);
  }

  return 1;
}
static int glms_emit_glsl_unop(GLMSEmit *emit, GLMSAST *ast, int indent) {
  GLMSAST* left = ast->as.unop.left;
  GLMSAST* right = ast->as.unop.right;
  GLMSTokenType op = ast->as.unop.op;

  if (left != 0) {
    glms_emit_glsl_(emit, left, indent);
  }

  EMIT_TOKEN_TYPE(op, indent);

  if (right != 0) {
    glms_emit_glsl_(emit, right, 0);
  }

  return 1;
}
static int glms_emit_glsl_access(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_block(GLMSEmit *emit, GLMSAST *ast, int indent) {
  if (ast->as.block.body != 0) {
    glms_emit_glsl_(emit, ast->as.block.body, indent);
  }
  return 1;
}
static int glms_emit_glsl_import(GLMSEmit *emit, GLMSAST *ast, int indent) {
  GLMSAST result = glms_eval(&emit->env->eval, *ast, &emit->env->stack);
  if (result.type != GLMS_AST_TYPE_STACK_PTR) return 0;
  GLMSAST* ptr = result.as.stackptr.ptr;
  if (!ptr) return 0;
  if (ptr->as.stack.env == 0 || ptr->as.stack.env == emit->env) return 0;
  if (ptr->as.stack.env->root != 0) {
    return glms_emit_glsl_(emit, ptr->as.stack.env->root, indent);
  }
  
  return glms_emit_glsl_as_is(emit, ptr);
}
static int glms_emit_glsl_layout(GLMSEmit *emit, GLMSAST *ast, int indent) {
  EMIT_APPEND("layout(");
  if (ast->children != 0) {
    for (int64_t i = 0; i < ast->children->length; i++) {
      GLMSAST *child = ast->children->items[i];
      glms_emit_glsl_(emit, child, 0);

      if ((i+1) < ast->children->length) {
        EMIT_APPEND(", ");
      }
    }
  }
  EMIT_APPEND(")");
  if (ast->as.layout.right != 0) {
    EMIT_APPEND(" ");
    glms_emit_glsl_(emit, ast->as.layout.right, indent);
  }

  return 1;
}
static int glms_emit_glsl_stack(GLMSEmit *emit, GLMSAST *ast, int indent) {
  return glms_emit_glsl_as_is(emit, ast);
}
static int glms_emit_glsl_for(GLMSEmit *emit, GLMSAST *ast, int indent) {
  GLMSAST* body = ast->as.forloop.body;

  EMIT_APPEND_INDENTED("for(", indent);
  if (ast->children != 0) {
    for (int64_t i = 0; i < ast->children->length; i++) {
      GLMSAST *child = ast->children->items[i];
      glms_emit_glsl_(emit, child, 0);

      if ((i+1) < ast->children->length) {
        EMIT_APPEND(", ");
      }
    }
  }
  EMIT_APPEND(")");
  EMIT_APPEND(" {\n");

  if (body != 0) {
    glms_emit_glsl_(emit, body, indent + INDENT_NUM);
  }

  EMIT_APPEND("\n");
  EMIT_APPEND_INDENTED("}", indent);

  return 1;
}
static int glms_emit_glsl_call(GLMSEmit *emit, GLMSAST *ast, int indent) {
  GLMSAST* left = ast->as.call.left;
  GLMSAST* right = ast->as.call.right;

  if (left != 0) {
    glms_emit_glsl_(emit, left, indent);
  }
  
  if (right != 0) {
    glms_emit_glsl_(emit, right, indent);
  }

  EMIT_APPEND("(");
  if (ast->children != 0) {
    for (int64_t i = 0; i < ast->children->length; i++) {
      GLMSAST *child = ast->children->items[i];
      glms_emit_glsl_(emit, child, 0);

      if ((i+1) < ast->children->length) {
        EMIT_APPEND(", ");
      }
    }
  }
  EMIT_APPEND(")");

  return 1;
}
static int glms_emit_glsl_func(GLMSEmit *emit, GLMSAST *ast, int indent) {
  const char* name = glms_ast_get_name(ast);
  if (!name) return 0;

  EMIT_APPEND_INDENTED(name, indent);

  EMIT_APPEND_INDENTED("(", indent);
  if (ast->children != 0) {
    for (int64_t i = 0; i < ast->children->length; i++) {
      GLMSAST *child = ast->children->items[i];
      glms_emit_glsl_(emit, child, indent);

      if ((i+1) < ast->children->length) {
        EMIT_APPEND_INDENTED(", ", indent);
      }
    }
  }
  EMIT_APPEND_INDENTED(") ", indent);

  EMIT_APPEND_INDENTED("{\n", indent);
  if (ast->as.func.body != 0) {
    glms_emit_glsl_(emit, ast->as.func.body, indent + INDENT_NUM);
  }
  EMIT_APPEND_INDENTED("\n}", indent);
  EMIT_APPEND("\n");
  
  
  return 1;
}
static int glms_emit_glsl_return(GLMSEmit *emit, GLMSAST *ast, int indent) {
  EMIT_APPEND_TYPE(ast->type);
  return 1;
}

static int glms_emit_glsl_id(GLMSEmit *emit, GLMSAST *ast, int indent) {
  const char *value = glms_ast_get_string_value(ast);
  if (!value)
    return 0;
  EMIT_APPEND_INDENTED(value, indent);
  return 1;
}

static int glms_emit_glsl_compound(GLMSEmit *emit, GLMSAST *ast, int indent) {
  if (!ast->children)
    return 0;

  for (int64_t i = 0; i < ast->children->length; i++) {
    GLMSAST *child = ast->children->items[i];
    glms_emit_glsl_(emit, child, indent);

    if (child->type != GLMS_AST_TYPE_FUNC &&
        child->type != GLMS_AST_TYPE_IMPORT &&
        child->type != GLMS_AST_TYPE_FOR &&
        child->type != GLMS_AST_TYPE_BLOCK &&
        child->type != GLMS_AST_TYPE_RAW_GLSL
    ) {
      EMIT_APPEND(";");
    }

    if ((i + 1) < ast->children->length) {
      EMIT_APPEND("\n");
    }
  }

  return 1;
}

static int glms_emit_glsl_(GLMSEmit *emit, GLMSAST *ast, int indent) {
   if (!emit || !ast || emit->initialized == false)
    return 0;

  if (ast->flags != 0) {
    for (int64_t i = 0; i < ast->flags->length; i++) {
      glms_emit_glsl_(emit, ast->flags->items[i], indent);
      EMIT_APPEND(" ");
    }
  }
  
  switch (ast->type) {
  case GLMS_AST_TYPE_COMPOUND:
    return glms_emit_glsl_compound(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_ID:
    return glms_emit_glsl_id(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_STACK_PTR:
    return glms_emit_glsl_stack_ptr(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_FUNC_OVERLOAD_PTR:
    return glms_emit_glsl_func_overload_ptr(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_STRING:
    return glms_emit_glsl_string(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_RAW_GLSL:
    return glms_emit_glsl_raw_glsl(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_CHAR:
    return glms_emit_glsl_char(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_NUMBER:
    return glms_emit_glsl_number(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_BOOL:
    return glms_emit_glsl_bool(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_ARRAY:
    return glms_emit_glsl_array(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_VEC2:
    return glms_emit_glsl_vec2(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_VEC3:
    return glms_emit_glsl_vec3(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_VEC4:
    return glms_emit_glsl_vec4(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_MAT3:
    return glms_emit_glsl_mat3(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_MAT4:
    return glms_emit_glsl_mat4(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_TYPEDEF:
    return glms_emit_glsl_typedef(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_OBJECT:
    return glms_emit_glsl_object(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_STRUCT:
    return glms_emit_glsl_struct(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_ENUM:
    return glms_emit_glsl_enum(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_BINOP:
    return glms_emit_glsl_binop(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_UNOP:
    return glms_emit_glsl_unop(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_ACCESS:
    return glms_emit_glsl_access(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_BLOCK:
    return glms_emit_glsl_block(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_IMPORT:
    return glms_emit_glsl_import(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_LAYOUT:
    return glms_emit_glsl_layout(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_STACK:
    return glms_emit_glsl_stack(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_FOR:
    return glms_emit_glsl_for(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_CALL:
    return glms_emit_glsl_call(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_FUNC:
    return glms_emit_glsl_func(emit, ast, indent);
    break;
  case GLMS_AST_TYPE_RETURN:
    return glms_emit_glsl_return(emit, ast, indent);
    break;
  default: {
    EMIT_APPEND_TYPE(ast->type);
  }; break;
  }

  return 1;

  
}

int glms_emit_glsl(GLMSEmit *emit, GLMSAST *ast) {
  return glms_emit_glsl_(emit, ast, 0);
}
