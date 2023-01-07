#ifndef GLMS_TOKEN_H
#define GLMS_TOKEN_H
#include <glms/macros.h>
#include <glms/string_view.h>
#include <stdbool.h>

#define GLMS_FOREACH_TOKEN_TYPE(TOK)                                           \
  TOK(GLMS_TOKEN_TYPE_NONE)                                                    \
  TOK(GLMS_TOKEN_TYPE_EOF)                                                     \
  TOK(GLMS_TOKEN_TYPE_LBRACE)                                                  \
  TOK(GLMS_TOKEN_TYPE_RBRACE)                                                  \
  TOK(GLMS_TOKEN_TYPE_LBRACKET)                                                \
  TOK(GLMS_TOKEN_TYPE_RBRACKET)                                                \
  TOK(GLMS_TOKEN_TYPE_LPAREN)                                                  \
  TOK(GLMS_TOKEN_TYPE_RPAREN)                                                  \
  TOK(GLMS_TOKEN_TYPE_SEMI)                                                    \
  TOK(GLMS_TOKEN_TYPE_COMMA)                                                   \
  TOK(GLMS_TOKEN_TYPE_COLON)                                                   \
  TOK(GLMS_TOKEN_TYPE_DOT)                                                     \
  TOK(GLMS_TOKEN_TYPE_ID)                                                      \
  TOK(GLMS_TOKEN_TYPE_STRING)                                                  \
  TOK(GLMS_TOKEN_TYPE_TEMPLATE_STRING)                                         \
  TOK(GLMS_TOKEN_TYPE_NUMBER)                                                  \
  TOK(GLMS_TOKEN_TYPE_INT)                                                     \
  TOK(GLMS_TOKEN_TYPE_FLOAT)                                                   \
  TOK(GLMS_TOKEN_TYPE_ADD)                                                     \
  TOK(GLMS_TOKEN_TYPE_ADD_ADD)                                                 \
  TOK(GLMS_TOKEN_TYPE_ADD_EQUALS)                                              \
  TOK(GLMS_TOKEN_TYPE_AND_AND)                                                 \
  TOK(GLMS_TOKEN_TYPE_PIPE_PIPE)                                               \
  TOK(GLMS_TOKEN_TYPE_MUL)                                                     \
  TOK(GLMS_TOKEN_TYPE_MUL_EQUALS)                                              \
  TOK(GLMS_TOKEN_TYPE_SUB)                                                     \
  TOK(GLMS_TOKEN_TYPE_SUB_SUB)                                                 \
  TOK(GLMS_TOKEN_TYPE_SUB_EQUALS)                                              \
  TOK(GLMS_TOKEN_TYPE_DIV)                                                     \
  TOK(GLMS_TOKEN_TYPE_DIV_EQUALS)                                              \
  TOK(GLMS_TOKEN_TYPE_PERCENT)                                                 \
  TOK(GLMS_TOKEN_TYPE_EQUALS)                                                  \
  TOK(GLMS_TOKEN_TYPE_EQUALS_EQUALS)                                           \
  TOK(GLMS_TOKEN_TYPE_GT)                                                      \
  TOK(GLMS_TOKEN_TYPE_LT)                                                      \
  TOK(GLMS_TOKEN_TYPE_GTE)                                                     \
  TOK(GLMS_TOKEN_TYPE_LTE)                                                     \
  TOK(GLMS_TOKEN_TYPE_EXCLAM)                                                  \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_NULL)                                            \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_IMPORT)                                          \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_AS)                                              \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_TYPEDEF)                                         \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_USER_TYPE)                                       \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_CONST)                                           \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_LET)                                             \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_STRUCT)                                          \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_VEC2)                                            \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_VEC3)                                            \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_VEC4)                                            \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_MAT3)                                            \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_MAT4)                                            \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_ENUM)                                            \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_STRING)                                          \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_NUMBER)                                          \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_INT)                                             \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_FLOAT)                                           \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_ARRAY)                                           \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_OBJECT)                                          \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_FOR)                                             \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_SWITCH)                                          \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_CASE)                                            \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_BREAK)                                           \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_WHILE)                                           \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_FALSE)                                           \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_TRUE)                                            \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_BOOL)                                            \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_IF)                                              \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_ELSE)                                            \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_FUNCTION)                                        \
  TOK(GLMS_TOKEN_TYPE_SPECIAL_RETURN)

typedef enum { GLMS_FOREACH_TOKEN_TYPE(GLMS_GENERATE_ENUM) } GLMSTokenType;

static const char *GLMS_TOKEN_TYPE_STR[] = {
    GLMS_FOREACH_TOKEN_TYPE(GLMS_GENERATE_STRING)};

typedef struct {
  GLMSTokenType type;
  GLMSStringView value;
  char c;
} GLMSToken;

bool glms_token_type_is_flag(GLMSTokenType type);
#endif
