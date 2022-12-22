#ifndef JSCRIPT_AST_H
#define JSCRIPT_AST_H
#include <jscript/string_view.h>
#include <jscript/token.h>
#include <jscript/buffer.h>
#include <jscript/list.h>
#include <jscript/fptr.h>
#include <jscript/macros.h>

#define JSCRIPT_FOREACH_AST_TYPE(TOK)           \
  TOK(JSCRIPT_AST_TYPE_EOF)                     \
  TOK(JSCRIPT_AST_TYPE_NOOP)\
  TOK(JSCRIPT_AST_TYPE_COMPOUND)\
  TOK(JSCRIPT_AST_TYPE_ID)\
  TOK(JSCRIPT_AST_TYPE_STRING)\
  TOK(JSCRIPT_AST_TYPE_NUMBER)\
  TOK(JSCRIPT_AST_TYPE_BINOP)\
  TOK(JSCRIPT_AST_TYPE_UNOP)\
  TOK(JSCRIPT_AST_TYPE_BLOCK)\
  TOK(JSCRIPT_AST_TYPE_CALL)\
  TOK(JSCRIPT_AST_TYPE_FUNC)\
  TOK(JSCRIPT_AST_TYPE_RETURN)

typedef enum {
  JSCRIPT_FOREACH_AST_TYPE(JSCRIPT_GENERATE_ENUM)
} JSCRIPTASTType;

static const char *JSCRIPT_AST_TYPE_STR[] = {
    JSCRIPT_FOREACH_AST_TYPE(JSCRIPT_GENERATE_STRING)
};

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
      JAST* id;
      JAST* body;
    } func;

    struct {
      JSCRIPTTokenType op;
      JAST* right;
    } unop;

    struct {
      JSCRIPTTokenType op;
      JAST* body;
      JAST* expr;
      JAST* next;
    } block;

  } as;

  JSCRIPTASTType type;
  struct JSCRIPT_JSCRIPTAST_LIST_STRUCT* children;
  JSCRIPTFPTR fptr;
} JSCRIPTAST;

JSCRIPT_DEFINE_BUFFER(JSCRIPTAST);
JSCRIPT_DEFINE_LIST(JSCRIPTAST);

JSCRIPTAST* jscript_ast_push(JSCRIPTAST* parent, JSCRIPTAST* child);

bool jscript_ast_is_iterable(JSCRIPTAST* ast);

const char* jscript_ast_get_name(JSCRIPTAST* ast);

const char* jscript_ast_to_string(JSCRIPTAST* ast);

bool jscript_ast_is_truthy(JSCRIPTAST* ast);

#define JSCRIPTAST_VALUE(ast) (ast->as.number.value)

#endif
