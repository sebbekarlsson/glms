#ifndef GLMS_AST_H
#define GLMS_AST_H
#include <arena/arena.h>
#include <cglm/struct.h>
#include <fastjson/json.h>
#include <glms/allocator.h>
#include <glms/ast_type.h>
#include <glms/buffer.h>
#include <glms/fptr.h>
#include <glms/iterator.h>
#include <glms/list.h>
#include <glms/macros.h>
#include <glms/string_view.h>
#include <glms/token.h>
#include <glms/type.h>
#include <hashy/hashy.h>
#include <stdbool.h>
#include <vec3/vec3.h>

#define GLMS_AST_OPERATOR_OVERLOAD_CAP 24

struct GLMS_ENV_STRUCT;
struct GLMS_EVAL_STRUCT;
struct GLMS_STACK_STRUCT;

struct GLMS_BUFFER_GLMSAST;
struct GLMS_GLMSAST_LIST_STRUCT;

struct GLMS_AST_STRUCT;

#define JAST struct GLMS_AST_STRUCT

typedef void (*GLMSASTContructor)(struct GLMS_EVAL_STRUCT* eval,
                                  struct GLMS_STACK_STRUCT* stack,
                                  struct GLMS_BUFFER_GLMSAST* args,
                                  struct GLMS_AST_STRUCT* self);

typedef int (*GLMSASTSwizzle)(struct GLMS_EVAL_STRUCT* eval,
                              struct GLMS_STACK_STRUCT* stack,
                              struct GLMS_AST_STRUCT* ast,
                              struct GLMS_AST_STRUCT* accessor,
                              struct GLMS_AST_STRUCT* out);

typedef char* (*GLMSASTToString)(struct GLMS_AST_STRUCT* ast,
                                 GLMSAllocator alloc,
                                 struct GLMS_ENV_STRUCT* env);

typedef void (*GLMSASTDestructor)(struct GLMS_AST_STRUCT* ast);

typedef int (*GLMSASTAtomFunc)(struct GLMS_AST_STRUCT* ast,
                               struct GLMS_BUFFER_GLMSAST* out);

typedef int (*GLMSASTOperatorOverload)(struct GLMS_EVAL_STRUCT* eval,
                                       struct GLMS_STACK_STRUCT* stack,
                                       struct GLMS_AST_STRUCT* left,
                                       struct GLMS_AST_STRUCT* right,
                                       struct GLMS_AST_STRUCT* out);

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
      GLMSStringView value;
      JAST* id;
    } import;

    struct {
      JAST* factor;
      JAST* id;
    } tdef;

    struct {
      GLMSStringView value;
      char* heap;
    } string;

    struct {
      char c;
    } character;

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
      char* name;
      GLMSFunctionSignatureBuffer signatures;
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

    struct {
      struct GLMS_ENV_STRUCT* env;
    } stack;

    struct {
      GLMSIterator it;
      void* state;
    } iterator;

    struct {
      int idx;
      JAST* ptr;
    } stackptr;

    bool boolean;

    Vector2 v2;
    Vector3 v3;
    Vector4 v4;

    mat3s m3;
    mat4s m4;

  } as;

  GLMSASTType type;
  HashyMap props;
  struct GLMS_GLMSAST_LIST_STRUCT* children;
  struct GLMS_GLMSAST_LIST_STRUCT* flags;
  GLMSASTOperatorOverload op_overloads[GLMS_AST_OPERATOR_OVERLOAD_CAP];
  GLMSFPTR fptr;
  JSON* json;
  void* ptr;
  char* string_rep;
  GLMSASTContructor constructor;
  GLMSASTSwizzle swizzle;
  GLMSASTToString to_string;
  GLMSASTDestructor destructor;
  GLMSASTAtomFunc get_atoms;
  GLMSIteratorNext iterator_next;
  char* typename;
  JAST* value_type;
  JAST* result;
  float* floats;
  ArenaRef ref;
  bool keep;
  bool is_tmp;
  bool is_heap;
  bool constructed;
  bool is_reserved;
  struct GLMS_ENV_STRUCT* env_ref;
} GLMSAST;

GLMS_DEFINE_BUFFER(GLMSAST);
GLMS_DEFINE_LIST(GLMSAST);

GLMSAST* glms_ast_push(GLMSAST* parent, GLMSAST* child);
GLMSAST* glms_ast_push_flag(GLMSAST* parent, GLMSAST* flag);

bool glms_ast_is_iterable(GLMSAST* ast);

int glms_ast_iterate(struct GLMS_ENV_STRUCT* env, GLMSAST* ast,
                     GLMSIterator* it, GLMSAST* out);

const char* glms_ast_get_name(GLMSAST* ast);

const char* glms_ast_get_string_value(GLMSAST* ast);

char* glms_ast_to_string(GLMSAST ast, GLMSAllocator alloc,
                         struct GLMS_ENV_STRUCT* env);

bool glms_ast_is_truthy(GLMSAST ast);

bool glms_ast_compare_equals_equals(GLMSAST a, GLMSAST b);
bool glms_ast_compare_gt(GLMSAST a, GLMSAST b);
bool glms_ast_compare_gte(GLMSAST a, GLMSAST b);
bool glms_ast_compare_lt(GLMSAST a, GLMSAST b);
bool glms_ast_compare_lte(GLMSAST a, GLMSAST b);

float glms_ast_get_number_by_key(GLMSAST* ast, const char* key);

GLMSAST* glms_ast_object_set_property(GLMSAST* obj, const char* key,
                                      GLMSAST* value);

GLMSAST* glms_ast_access_by_index(GLMSAST* ast, int64_t index,
                                  struct GLMS_ENV_STRUCT* env);
GLMSAST* glms_ast_access_by_key(GLMSAST* ast, const char* key,
                                struct GLMS_ENV_STRUCT* env);

int64_t glms_ast_array_get_length(GLMSAST* ast);

int glms_ast_get_type(GLMSAST ast, GLMSAST* out);

GLMSAST* glms_ast_copy(GLMSAST src, struct GLMS_ENV_STRUCT* env);

void glms_ast_destructor(GLMSAST* ast);

bool glms_ast_is_vector(GLMSAST* ast);

void glms_ast_keep(GLMSAST* ast);

GLMSAST* glms_ast_get_property(GLMSAST* ast, const char* key);

GLMSAST* glms_ast_register_function(struct GLMS_ENV_STRUCT* env, GLMSAST* ast,
                                    const char* name, GLMSFPTR fptr);

GLMSAST glms_ast_op_add_add(GLMSAST* a);
GLMSAST glms_ast_op_sub_sub(GLMSAST* a);

GLMSAST glms_ast_op_add(GLMSAST a, GLMSAST b);
GLMSAST glms_ast_op_add_eq(GLMSAST* a, GLMSAST b);
GLMSAST glms_ast_op_sub_eq(GLMSAST* a, GLMSAST b);
GLMSAST glms_ast_op_mul_eq(GLMSAST* a, GLMSAST b);
GLMSAST glms_ast_op_div_eq(GLMSAST* a, GLMSAST b);

GLMSAST glms_ast_op_sub(GLMSAST a, GLMSAST b);
GLMSAST glms_ast_op_mul(GLMSAST a, GLMSAST b);
GLMSAST glms_ast_op_mod(GLMSAST a, GLMSAST b);
GLMSAST glms_ast_op_div(GLMSAST a, GLMSAST b);
GLMSAST glms_ast_op_eq(GLMSAST a, GLMSAST b);
GLMSAST glms_ast_op_and_and(GLMSAST a, GLMSAST b);
GLMSAST glms_ast_op_pipe_pipe(GLMSAST a, GLMSAST b);
GLMSAST glms_ast_op_lt(GLMSAST a, GLMSAST b);
GLMSAST glms_ast_op_gt(GLMSAST a, GLMSAST b);
GLMSAST glms_ast_op_lte(GLMSAST a, GLMSAST b);
GLMSAST glms_ast_op_gte(GLMSAST a, GLMSAST b);
GLMSAST glms_ast_assign(GLMSAST* a, GLMSAST b, struct GLMS_EVAL_STRUCT* eval,
                        struct GLMS_STACK_STRUCT* stack);

GLMSAST* glms_ast_get_ptr(GLMSAST a);

GLMSAST* glms_ast_register_operator_overload(struct GLMS_ENV_STRUCT* env,
                                             GLMSAST* ast, GLMSTokenType op,
                                             GLMSASTOperatorOverload func);

GLMSASTOperatorOverload glms_ast_get_op_overload(GLMSAST ast, GLMSTokenType op,
                                                 struct GLMS_ENV_STRUCT* env);

GLMSAST* glms_ast_register_func_overload(struct GLMS_ENV_STRUCT* env,
                                         GLMSAST* ast, const char* name,
                                         GLMSFPTR func);

GLMSFPTR glms_ast_get_func_overload(GLMSAST ast, const char* name);

float glms_ast_number(GLMSAST ast);

char* glms_ast_to_string_debug(GLMSAST ast);

typedef struct {
  bool have_detail;
} GLMSDocstringGenerator;

char* glms_ast_generate_docstring(GLMSAST ast, const char* name,
                                  const char* suffix, int depth,
                                  GLMSDocstringGenerator* gen);

int glms_ast_get_atoms(GLMSAST ast, GLMSASTBuffer* out);

GLMSAST* glms_ast_from_json(struct GLMS_ENV_STRUCT* env, JSON* v);

#define GLMSAST_VALUE(ast) (ast->as.number.value)

#endif
