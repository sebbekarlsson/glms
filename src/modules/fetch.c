#include "fastjson/json.h"
#include "fastjson/options.h"
#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/env.h"
#include "glms/eval.h"
#include "glms/type.h"
#include <glms/modules/fetch.h>
#include <curl/curl.h>
#include <glms/macros.h>

typedef struct {
  char* data;
  int64_t len;
  int64_t code;
} GLMSFetchResponse;

char *glms_response_to_string(GLMSAST *ast, GLMSAllocator alloc, GLMSEnv *env) {
  GLMSFetchResponse* response = ast->ptr ? (GLMSFetchResponse*)ast->ptr : 0;

  if (response != 0 && response->data == 0) return alloc.strdup(alloc.user_ptr, "(null)");

  char tmp[256];
  sprintf(tmp, "Response<%ld>", response ? response->code : 0);

  return alloc.strdup(alloc.user_ptr, tmp);
}

void glms_response_destructor(GLMSAST *ast) {
  if (!ast)
    return;

  if (!ast->ptr)
    return;

  GLMSFetchResponse* response = (GLMSFetchResponse*)ast->ptr;

  if (response->data != 0) {
    free(response->data);
    response->data = 0;
  }

  response->code = 0;
  response->len = 0;

  free(response);

  ast->ptr = 0;
}

static size_t writefunc(void *ptr, size_t size, size_t nmemb, GLMSFetchResponse *response)
{
    size_t new_len = response->len + size*nmemb;
    response->data = realloc(response->data, new_len+1);

    if (response->data == NULL)
    {
      GLMS_WARNING_RETURN(0, stderr, "failed to realloc response response.\n");
    }

    memcpy(response->data+response->len, ptr, size*nmemb);
    response->data[new_len] = '\0';
    response->len = new_len;

    return size*nmemb;
}

int glms_fptr_fetch(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                    GLMSStack *stack, GLMSAST *out) {


  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_STRING }, 1, args)) return 0;
  CURL *curl;
  CURLcode res;

  curl = curl_easy_init();

  if (!curl) GLMS_WARNING_RETURN(0, stderr, "Failed to initialize curl.\n");

  const char* path = glms_ast_get_string_value(&args->items[0]);

  GLMSFetchResponse* response = NEW(GLMSFetchResponse);

  curl_easy_setopt(curl, CURLOPT_URL, path);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
  res = curl_easy_perform(curl);

  long http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  response->code = http_code;
  curl_easy_cleanup(curl);


  GLMSAST* new_ast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_STRUCT, true);
  new_ast->ptr = response;

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_ast };

  return 1;
}

int glms_response_fptr_text(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                            GLMSStack *stack, GLMSAST *out) {
  if (!ast->ptr) GLMS_WARNING_RETURN(0, stderr, "ptr == null.\n");

  
  GLMSFetchResponse* response = (GLMSFetchResponse*)ast->ptr;


  GLMSAST* string_ast = glms_env_new_ast_string(eval->env, response->data, true);
  
  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = string_ast };

  return 1;
}

int glms_response_fptr_json(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                            GLMSStack *stack, GLMSAST *out) {
  if (!ast->ptr) GLMS_WARNING_RETURN(0, stderr, "ptr == null.\n");

  
  GLMSFetchResponse* response = (GLMSFetchResponse*)ast->ptr;

  const char* text = response->data;

  if (!text) GLMS_WARNING_RETURN(0, stderr, "cannot parse null.\n");


  JSONOptions joptions = {0};
  joptions.optimized_strings = true;
  JSON* j = json_parse(text, &joptions);

  if (!j) GLMS_WARNING_RETURN(0, stderr, "failed to parse json.\n");

  GLMSAST* obj_ast = glms_ast_from_json(eval->env, j);
  
  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = obj_ast };

  return 1;
}

int glms_response_fptr_status(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                            GLMSStack *stack, GLMSAST *out) {
  if (!ast->ptr) GLMS_WARNING_RETURN(0, stderr, "ptr == null.\n");
  GLMSFetchResponse* response = (GLMSFetchResponse*)ast->ptr;


  GLMSAST* number_ast = glms_env_new_ast_number(eval->env, (float)response->code, true);
  
  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = number_ast };

  return 1;
}

void glms_response_constructor(GLMSEval *eval, GLMSStack *stack,
                               GLMSASTBuffer *args, GLMSAST *self) {

  self->constructor = glms_response_constructor;
  glms_ast_register_function(eval->env, self, "text", glms_response_fptr_text);
  glms_ast_register_function(eval->env, self, "json", glms_response_fptr_json);
  glms_ast_register_function(eval->env, self, "data", glms_response_fptr_json);
  glms_ast_register_function(eval->env, self, "status", glms_response_fptr_status);
  
}

void glms_fetch(GLMSEnv *env) {

  GLMSAST* t = glms_env_new_ast(env, GLMS_AST_TYPE_STRUCT, false);
  t->constructor = glms_response_constructor;
  glms_env_register_type(env, "response", t, glms_response_constructor, 0, glms_response_to_string, glms_response_destructor);

  glms_env_register_function(env, "fetch", glms_fptr_fetch);
  glms_env_register_function_signature(env, 0, "fetch", (GLMSFunctionSignature){
      .return_type = (GLMSType){ .typename = "response" },
      .args = (GLMSType[]) {
	(GLMSType){ GLMS_AST_TYPE_STRING, .valuename = "URL" }
      },
      .args_length = 1
  });
}
