#include "glms/ast_type.h"
#include "glms/builtin.h"
#include "glms/macros.h"
#include "glms/stack.h"
#include "glms/string_view.h"
#include <gimg/gimg.h>
#include <glms/ast.h>
#include <glms/modules/image.h>
#include <glms/env.h>
#include <glms/eval.h>
#include <text/text.h>

int glms_struct_image_fptr_get_pixel(GLMSEval *eval, GLMSAST *ast,
				     GLMSASTBuffer *args, GLMSStack *stack, GLMSAST* out) {
  if (!args || args->length < 2)
    return 0;

  if (!ast->ptr)
    return 0;

  GIMG *gimg = (GIMG *)ast->ptr;

  int x = (int)(glms_eval(eval, args->items[0], stack)).as.number.value;
  int y = (int)(glms_eval(eval, args->items[1], stack)).as.number.value;

  Vector4 pixel = gimg_get_pixel_vec4(gimg, x, y);


  GLMSAST* new_ast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_VEC4, true);
  new_ast->as.v4 = pixel;
  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_ast };
  return 1;
}

int glms_struct_image_fptr_set_pixel(GLMSEval *eval, GLMSAST *ast,
                                          GLMSASTBuffer *args, GLMSStack *stack, GLMSAST* out) {
  if (!args || args->length < 3)
    return 0;

  if (!ast->ptr)
    return 0;

  GIMG *gimg = (GIMG *)ast->ptr;

  int x = (int)(glms_eval(eval, args->items[0], stack)).as.number.value;
  int y = (int)(glms_eval(eval, args->items[1], stack)).as.number.value;

  Vector4 pixel = glms_eval(eval, args->items[2], stack).as.v4;

  gimg_set_pixel_vec4(gimg, x, y, pixel);

  return 0;
}

int glms_struct_image_fptr_make(GLMSEval *eval, GLMSAST *ast,
                                     GLMSASTBuffer *args, GLMSStack *stack, GLMSAST* out) {
  if (!args || args->length < 2)
    GLMS_WARNING_RETURN(0, stderr, "not enough arguments.\n");
  //  if (ast->ptr) return 0; // already made

  GIMG *gimg = ast->ptr ? ast->ptr : NEW(GIMG);

  int w = (int)(glms_eval(eval, args->items[0], stack)).as.number.value;
  int h = (int)(glms_eval(eval, args->items[1], stack)).as.number.value;

  if (w <= 0 || h <= 0) GLMS_WARNING_RETURN(0, stderr, "Invalid dimensions for image.\n");

  GLMSAST *imgast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_STRUCT, true);
  if (!gimg_make(gimg, w, h, 4)) {
    GLMS_WARNING_RETURN(0, stderr, "Failed to create image.\n");
  }

  imgast->ptr = gimg;

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = imgast };

  return 1;
}

int glms_struct_image_fptr_save(GLMSEval *eval, GLMSAST *ast,
                                     GLMSASTBuffer *args, GLMSStack *stack, GLMSAST* out) {
  if (!args || args->length <= 0)
    return 0;
  if (!ast->ptr)
    GLMS_WARNING_RETURN(0, stderr, "Image not initialized (ptr = null).\n");

  GIMG *gimg = (GIMG *)ast->ptr;

  GLMSAST arg0 = glms_eval(eval, args->items[0], stack);

  int ok = 0;

  if (arg0.type == GLMS_AST_TYPE_STRING) {
    const char *strval = glms_string_view_get_value(&arg0.as.string.value);

    if (strval) {
      ok = gimg_save(*gimg, strval, true);
    }
  }

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_BOOL, .as.boolean = ok ? true : false };

  return 1;
}


int glms_struct_image_fptr_load(GLMSEval *eval, GLMSAST *ast,
                                     GLMSASTBuffer *args, GLMSStack *stack, GLMSAST* out) {
  if (!args || args->length <= 0)
    return 0;


  GIMG* gimg = NEW(GIMG);
  if (args != 0 && args->length > 0) {
     ast->ptr = ast->ptr ? ast->ptr : NEW(GIMG);
    GLMSAST arg0 = glms_eval(eval, args->items[0], stack);

    if (arg0.type == GLMS_AST_TYPE_STRING) {
      gimg_read_from_path(gimg, glms_string_view_get_value(&arg0.as.string.value));
    }
  }

  GLMSAST* new_ast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_STRUCT, true);
  new_ast->ptr = gimg;
  new_ast->constructor = glms_struct_image_constructor;
  glms_struct_image_constructor(eval, stack, 0, new_ast);

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_ast };

  return 1;
}


int glms_struct_image_fptr_shade(GLMSEval *eval, GLMSAST *ast,
                                      GLMSASTBuffer *args, GLMSStack *stack, GLMSAST* out) {

  if (!args || args->length <= 0)
    return 0;
  if (!ast->ptr)
    GLMS_WARNING_RETURN(0, stderr, "Image not initialized (ptr = null).\n");

  GIMG *gimg = (GIMG *)ast->ptr;

  GLMSAST arg0 = args->items[0];

  const char *signature[] = {"uv", "fragCoord", "resolution"};

  GLMSAST *uv_ast = glms_env_new_ast_vec3(eval->env, VEC31(0), false);
  GLMSAST *coord_ast = glms_env_new_ast_vec3(eval->env, VEC31(0), false);
  GLMSAST *res_ast = glms_env_new_ast_vec3(
      eval->env, VEC3(gimg->width, gimg->height, 0), false);

  GLMSAST *signature_values[] = {uv_ast, coord_ast, res_ast};

  GLMSAST call_ast = (GLMSAST){ .type = GLMS_AST_TYPE_CALL };//glms_env_new_ast(eval->env, GLMS_AST_TYPE_CALL, false);
  call_ast.as.call.func = &arg0;

  //GLMSStack tmp_stack = {0};
  // glms_stack_init(&tmp_stack);
  // glms_stack_copy(*stack, &tmp_stack);

  for (int j = 0; j < 3; j++) {
    glms_stack_push(stack, signature[j], signature_values[j]);
  }

  for (int x = 0; x < gimg->width; x++) {
    for (int y = 0; y < gimg->height; y++) {
      float u = (float)x / (float)gimg->width;
      float v = (float)y / (float)gimg->height;
      uv_ast->as.v3 = VEC3(u, v, 0);
      coord_ast->as.v3 = VEC3(x, y, 0);

      GLMSAST result = glms_eval(eval, call_ast, stack);

      result = glms_eval(eval, result, stack);


      if (result.type == GLMS_AST_TYPE_VEC4) {
	Vector4 translated = result.as.v4;
	translated.x *= 255.0f;
	translated.y *= 255.0f;
	translated.z *= 255.0f;
	translated.w *= 255.0f;
        if (!gimg_set_pixel_vec4(gimg, x, y, translated)) {
	  goto done;
	}
      }
    }
  }

 done:

  // glms_stack_clear_trash(&tmp_stack);
  //glms_stack_clear(&tmp_stack);

  return 1;
}

void glms_struct_image_constructor(GLMSEval *eval, GLMSStack *stack,
                                       GLMSASTBuffer *args, GLMSAST *self) {
  if (!self) return;
  GLMSAST *ast = self;
  ast->value_type = ast;
  ast->constructor = glms_struct_image_constructor;
 

  glms_ast_register_function(eval->env, ast, "getPixel",
                             glms_struct_image_fptr_get_pixel);
  glms_ast_register_function(eval->env, ast, "setPixel",
                             glms_struct_image_fptr_set_pixel);

  glms_ast_register_function(eval->env, ast, "load",
                             glms_struct_image_fptr_load);
  
  glms_ast_register_function(eval->env, ast, "make",
                             glms_struct_image_fptr_make);
  glms_ast_register_function(eval->env, ast, "save",
                             glms_struct_image_fptr_save);
  glms_ast_register_function(eval->env, ast, "shade",
                             glms_struct_image_fptr_shade);

  glms_env_register_function_signature(
    eval->env,
    ast,
    "load",
    (GLMSFunctionSignature){
      .return_type = (GLMSType){GLMS_AST_TYPE_STRUCT},
      .args = (GLMSType[]){ (GLMSType){ GLMS_AST_TYPE_STRING } },
      .args_length = 1
    }
  );

  glms_env_register_function_signature(
    eval->env,
    ast,
    "shade",
    (GLMSFunctionSignature){
      .return_type = (GLMSType){GLMS_AST_TYPE_BOOL},
      .args = (GLMSType[]){ (GLMSType){ GLMS_AST_TYPE_FUNC } },
      .args_length = 1
    }
  );

  glms_env_register_function_signature(
    eval->env,
    ast,
    "make",
    (GLMSFunctionSignature){
      .return_type = (GLMSType){.typename = "image"},
      .args = (GLMSType[]){ (GLMSType){ GLMS_AST_TYPE_NUMBER, .valuename = "width" }, (GLMSType){ GLMS_AST_TYPE_NUMBER, .valuename = "height" } },
      .args_length = 2
    }
  );

  glms_env_register_function_signature(
    eval->env,
    ast,
    "setPixel",
    (GLMSFunctionSignature){
      .return_type = (GLMSType){ GLMS_AST_TYPE_VOID },
      .args = (GLMSType[]){ (GLMSType){ GLMS_AST_TYPE_VEC4, .valuename = "pixel" }},
      .args_length = 1
    }
  );

  glms_env_register_function_signature(
    eval->env,
    ast,
    "getPixel",
    (GLMSFunctionSignature){
      .return_type = (GLMSType){ GLMS_AST_TYPE_VEC4 },
      .args = (GLMSType[]){ (GLMSType){ GLMS_AST_TYPE_NUMBER, .valuename = "x" }, (GLMSType){ GLMS_AST_TYPE_NUMBER, .valuename = "y" } },
      .args_length = 2
    }
  );

  glms_env_register_function_signature(
    eval->env,
    ast,
    "save",
    (GLMSFunctionSignature){
      .return_type = (GLMSType){ GLMS_AST_TYPE_BOOL },
      .args = (GLMSType[]){ (GLMSType){ GLMS_AST_TYPE_STRING, .valuename = "filename" }},
      .args_length = 1
    }
  );
}

void glms_struct_image_destructor(GLMSAST *ast) {
  if (!ast)
    return;

  if (!ast->ptr)
    return;

  GIMG *gimg = (GIMG *)ast->ptr;

  gimg_free(gimg, true);

  ast->ptr = 0;
}

void glms_struct_image(GLMSEnv *env) {
  GLMSAST *ast = glms_env_new_ast(env, GLMS_AST_TYPE_STRUCT, false);
  ast->constructor = glms_struct_image_constructor;
  //  ast->to_string = glms_struct_image_to_string;
  //  ast->ptr = NEW(GIMG);

  glms_env_register_type(env, "image", ast, glms_struct_image_constructor, 0,
                         0,
                         glms_struct_image_destructor);
}
