#include <glms/type.h>
#include <text/text.h>

GLMS_IMPLEMENT_BUFFER(GLMSFunctionSignature);

GLMSFunctionSignature glms_make_signature(GLMSType ret_type, GLMSType *args,
                                          int args_length) {

  return (GLMSFunctionSignature){ .return_type = ret_type, .args = args, .args_length = args_length };
}


const char *glms_type_to_string(GLMSType t) {
  if (t.typename) return t.typename;
  return GLMS_AST_TYPE_STR[t.ast_type];
  
}
char* glms_function_signature_to_string(GLMSFunctionSignature signa, const char* func_name) {
  
  char* s = 0;
  text_append(&s, glms_type_to_string(signa.return_type));
  text_append(&s, " ");
  text_append(&s, func_name);
  text_append(&s, "(");

  if (signa.args != 0) {
    for (int i = 0; i < signa.args_length; i++) {
      text_append(&s, glms_type_to_string(signa.args[i]));

      if (signa.args[i].valuename) {
       text_append(&s, " ");
       text_append(&s, signa.args[i].valuename);
      }

      if (i < signa.args_length -1) {
	text_append(&s, ", ");
      }
    }
  }
  text_append(&s, ")");

  if (signa.description) {
    text_append(&s, " // ");
    text_append(&s, signa.description);
    text_append(&s, "\n");
  }

  return s;
}
