#ifndef GLMS_AST_H
#define GLMS_AST_H
#include <glms/string_view.h>
#include <glms/token.h>
#include <glms/buffer.h>
#include <glms/list.h>
#include <glms/fptr.h>
#include <glms/macros.h>
#include <hashy/hashy.h>
#include <vec3/vec3.h>

struct GLMS_ENV_STRUCT;
struct GLMS_EVAL_STRUCT;
struct GLMS_STACK_STRUCT;

#define GLMS_FOREACH_AST_TYPE(TOK)           \
  TOK(GLMS_AST_TYPE_EOF)                     \
  TOK(GLMS_AST_TYPE_NOOP)\
  TOK(GLMS_AST_TYPE_UNDEFINED)\
  TOK(GLMS_AST_TYPE_COMPOUND)\
  TOK(GLMS_AST_TYPE_ID)\
  TOK(GLMS_AST_TYPE_STRING)\
  TOK(GLMS_AST_TYPE_NUMBER)\
  TOK(GLMS_AST_TYPE_BOOL)\
  TOK(GLMS_AST_TYPE_ARRAY)\
  TOK(GLMS_AST_TYPE_VEC2)\
  TOK(GLMS_AST_TYPE_VEC3)\
  TOK(GLMS_AST_TYPE_VEC4)\
  TOK(GLMS_AST_TYPE_TYPEDEF)\
  TOK(GLMS_AST_TYPE_OBJECT)\
  TOK(GLMS_AST_TYPE_STRUCT)\
  TOK(GLMS_AST_TYPE_BINOP)\
  TOK(GLMS_AST_TYPE_UNOP)\
  TOK(GLMS_AST_TYPE_ACCESS)\
  TOK(GLMS_AST_TYPE_BLOCK)\
  TOK(GLMS_AST_TYPE_FOR)\
  TOK(GLMS_AST_TYPE_CALL)\
  TOK(GLMS_AST_TYPE_FUNC)\
  TOK(GLMS_AST_TYPE_RETURN)

typedef enum {
  GLMS_FOREACH_AST_TYPE(GLMS_GENERATE_ENUM)
} GLMSASTType;

static const char *GLMS_AST_TYPE_STR[] = {
    GLMS_FOREACH_AST_TYPE(GLMS_GENERATE_STRING)
};

struct GLMS_BUFFER_GLMSAST;
struct GLMS_GLMSAST_LIST_STRUCT;


struct GLMS_AST_STRUCT;

#define JAST struct GLMS_AST_STRUCT

typedef struct GLMS_AST_STRUCT* (*GLMSASTContructor)(
						     struct GLMS_EVAL_STRUCT* eval,
						     struct GLMS_STACK_STRUCT* stack,
						     struct GLMS_GLMSAST_LIST_STRUCT* args
						     );

typedef struct GLMS_AST_STRUCT *(*GLMSASTSwizzle)(
    struct GLMS_EVAL_STRUCT *eval, struct GLMS_STACK_STRUCT *stack,
    struct GLMS_AST_STRUCT *ast, struct GLMS_AST_STRUCT *accessor);

typedef const char* (*GLMSASTToString)(
						     struct GLMS_AST_STRUCT* ast
						     );

typedef struct GLMS_AST_STRUCT {

  union {
    struct {
      float value;
    } number;

    struct {
      GLMSStringView value;
      GLMSTokenType op;
      char* heap;
    } id;

    struct {
      JAST* factor;
      JAST* id;
    } tdef;

    struct {
      GLMSStringView value;
      char* heap;
    } string;

    struct {
      GLMSTokenType op;
      JAST* left;
      JAST* right;
    } binop;

    struct {
      JAST* left;
      JAST* right;
      JAST* func;
    } call;

    struct {
      JAST* left;
      JAST* right;
    } access;

    struct {
      JAST* id;
      JAST* body;
    } func;

    struct {
      JAST* body;
    } forloop;

    struct {
      GLMSTokenType op;
      JAST* right;
      JAST* left;
    } unop;

    struct {
      GLMSTokenType op;
      JAST* body;
      JAST* expr;
      JAST* next;
    } block;

    Vector2 v2;
    Vector3 v3;
    Vector3 v4;

  } as;

  GLMSASTType type;
  HashyMap props;
  struct GLMS_GLMSAST_LIST_STRUCT* children;
  struct GLMS_GLMSAST_LIST_STRUCT* flags;
  GLMSFPTR fptr;
  char* string_rep;
  GLMSASTContructor constructor;
  GLMSASTSwizzle swizzle;
  GLMSASTToString to_string;
} GLMSAST;

GLMS_DEFINE_BUFFER(GLMSAST);
GLMS_DEFINE_LIST(GLMSAST);

GLMSAST* glms_ast_push(GLMSAST* parent, GLMSAST* child);
GLMSAST* glms_ast_push_flag(GLMSAST* parent, GLMSAST* flag);

bool glms_ast_is_iterable(GLMSAST* ast);

const char* glms_ast_get_name(GLMSAST* ast);

const char* glms_ast_get_string_value(GLMSAST* ast);

const char* glms_ast_to_string(GLMSAST* ast);

bool glms_ast_is_truthy(GLMSAST* ast);

bool glms_ast_compare_equals_equals(GLMSAST* a, GLMSAST* b);
bool glms_ast_compare_gt(GLMSAST* a, GLMSAST* b);
bool glms_ast_compare_gte(GLMSAST* a, GLMSAST* b);
bool glms_ast_compare_lt(GLMSAST* a, GLMSAST* b);
bool glms_ast_compare_lte(GLMSAST* a, GLMSAST* b);

float glms_ast_get_number_by_key(GLMSAST* ast, const char* key);

GLMSAST* glms_ast_object_set_property(GLMSAST* obj, const char* key, GLMSAST* value);

GLMSAST* glms_ast_access_by_index(GLMSAST* ast, int64_t index, struct GLMS_ENV_STRUCT* env);
GLMSAST* glms_ast_access_by_key(GLMSAST* ast, const char* key, struct GLMS_ENV_STRUCT* env);

int64_t glms_ast_array_get_length(GLMSAST* ast);

GLMSAST* glms_ast_get_type(GLMSAST* ast);

GLMSAST* glms_ast_copy(GLMSAST src, struct GLMS_ENV_STRUCT* env);

void glms_ast_destructor(GLMSAST *ast);

bool glms_ast_is_vector(GLMSAST* ast);

#define GLMSAST_VALUE(ast) (ast->as.number.value)

#endif
