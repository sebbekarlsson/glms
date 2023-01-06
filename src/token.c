#include <glms/token.h>

bool glms_token_type_is_flag(GLMSTokenType type) {
  return type == GLMS_TOKEN_TYPE_SPECIAL_USER_TYPE ||
         type == GLMS_TOKEN_TYPE_SPECIAL_CONST ||
         type == GLMS_TOKEN_TYPE_SPECIAL_NUMBER ||
         type == GLMS_TOKEN_TYPE_SPECIAL_INT ||
         type == GLMS_TOKEN_TYPE_SPECIAL_FLOAT ||
         type == GLMS_TOKEN_TYPE_SPECIAL_ARRAY ||
         type == GLMS_TOKEN_TYPE_SPECIAL_OBJECT ||
         type == GLMS_TOKEN_TYPE_SPECIAL_STRING ||
         type == GLMS_TOKEN_TYPE_SPECIAL_LET ||
         type == GLMS_TOKEN_TYPE_SPECIAL_BOOL;
}
