#ifndef JSCRIPT_AST_H
#define JSCRIPT_AST_H
#include <jscript/string_view.h>
#include <jscript/token.h>
#include <jscript/buffer.h>
#include <jscript/list.h>
#include <jscript/fptr.h>

typedef enum {
JSCRIPT_AST_TYPE_EOF,
JSCRIPT_AST_TYPE_NOOP,
JSCRIPT_AST_TYPE_COMPOUND,
JSCRIPT_AST_TYPE_ID,
JSCRIPT_AST_TYPE_STRING,
JSCRIPT_AST_TYPE_NUMBER,
JSCRIPT_AST_TYPE_BINOP,
JSCRIPT_AST_TYPE_UNOP,
JSCRIPT_AST_TYPE_CALL,
JSCRIPT_AST_TYPE_FUNC
} JSCRIPTASTType;

struct JSCRIPT_BUFFER_JSCRIPTAST;
struct JSCRIPT_JSCRIPTAST_LIST_STRUCT;


struct JSCRIPT_AST_STRUCT;

#define JAST struct JSCRIPT_AST_STRUCT

typedef struct JSCRIPT_AST_STRUCT {

  union {
    struct {
      float value;
    } number;

    struct {
      JAST* flag;
      JSCRIPTStringView value;
    } id;

    struct {
      JSCRIPTStringView value;
    } string;

    struct {
      JSCRIPTTokenType op;
      JAST* left;
      JAST* right;
    } binop;

    struct {
      JAST* left;
      JAST* right;
    } call;

    struct {
      JSCRIPTTokenType op;
      JAST* right;
    } unop;
  } as;

  JSCRIPTASTType type;
  struct JSCRIPT_JSCRIPTAST_LIST_STRUCT* children;
  JSCRIPTFPTR fptr;
} JSCRIPTAST;

JSCRIPT_DEFINE_BUFFER(JSCRIPTAST);
JSCRIPT_DEFINE_LIST(JSCRIPTAST);

JSCRIPTAST* jscript_ast_push(JSCRIPTAST* parent, JSCRIPTAST* child);

bool jscript_ast_is_iterable(JSCRIPTAST* ast);

#define JSCRIPTAST_VALUE(ast) (ast->as.number.value)

#endif
