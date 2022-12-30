#include "glms/ast_type.h"
#include "glms/builtin.h"
#include "glms/stack.h"
#include <gimg/gimg.h>
#include <glms/ast.h>
#include <glms/env.h>
#include <glms/eval.h>
#include <text/text.h>

GLMSAST *glms_struct_image_fptr_get_pixel(GLMSEval *eval, GLMSAST *ast,
                                          GLMSASTList *args, GLMSStack *stack) {
  if (!args || args->length < 2)
    return ast;

  if (!ast->ptr)
    return ast;

  GIMG *gimg = (GIMG *)ast->ptr;

  int x = (int)GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
  int y = (int)GLMSAST_VALUE(glms_eval(eval, args->items[1], stack));

  Vector4 pixel = gimg_get_pixel_vec4(gimg, x, y);

  GLMSAST *v4 = glms_env_new_ast(eval->env, GLMS_AST_TYPE_VEC4, true);
  v4->as.v4 = pixel;

  return v4;
}

GLMSAST *glms_struct_image_fptr_set_pixel(GLMSEval *eval, GLMSAST *ast,
                                          GLMSASTList *args, GLMSStack *stack) {
  if (!args || args->length < 3)
    return ast;

  if (!ast->ptr)
    return ast;

  GIMG *gimg = (GIMG *)ast->ptr;

  int x = (int)GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
  int y = (int)GLMSAST_VALUE(glms_eval(eval, args->items[1], stack));

  Vector4 pixel = glms_eval(eval, args->items[2], stack)->as.v4;

  gimg_set_pixel_vec4(gimg, x, y, pixel);

  return ast;
}

GLMSAST *glms_struct_image_fptr_make(GLMSEval *eval, GLMSAST *ast,
                                     GLMSASTList *args, GLMSStack *stack) {
  if (!args || args->length < 2)
    return ast;
  //  if (ast->ptr) return ast; // already made

  GIMG *gimg = ast->ptr ? ast->ptr : NEW(GIMG);

  int w = (int)GLMSAST_VALUE(glms_eval(eval, args->items[0], stack));
  int h = (int)GLMSAST_VALUE(glms_eval(eval, args->items[1], stack));

  GLMSAST *imgast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_STRUCT, true);
  if (!gimg_make(gimg, w, h)) {
    GLMS_WARNING_RETURN(ast, stderr, "Failed to create image.\n");
  }

  imgast->ptr = gimg;

  return imgast;
}

GLMSAST *glms_struct_image_fptr_save(GLMSEval *eval, GLMSAST *ast,
                                     GLMSASTList *args, GLMSStack *stack) {
  if (!args || args->length <= 0)
    return ast;
  if (!ast->ptr)
    GLMS_WARNING_RETURN(ast, stderr, "Image not initialized (ptr = null).\n");

  glms_fptr_expect_args(
      eval, stack, args,
      (GLMSType[]){(GLMSType){.ast_type = GLMS_AST_TYPE_STRING}}, 1);

  GIMG *gimg = (GIMG *)ast->ptr;

  GLMSAST *arg0 = glms_eval(eval, args->items[0], stack);

  int ok = 0;

  if (arg0->type == GLMS_AST_TYPE_STRING) {
    const char *strval = glms_ast_get_string_value(arg0);

    if (strval) {
      ok = gimg_save(*gimg, strval);
    }
  }

  GLMSAST *boolast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_BOOL, true);
  boolast->as.boolean = ok ? true : false;
  return boolast;
}

GLMSAST *glms_struct_image_fptr_shade(GLMSEval *eval, GLMSAST *ast,
                                      GLMSASTList *args, GLMSStack *stack) {

  if (!args || args->length <= 0)
    return ast;
  if (!ast->ptr)
    GLMS_WARNING_RETURN(ast, stderr, "Image not initialized (ptr = null).\n");
  glms_fptr_expect_args(
      eval, stack, args,
      (GLMSType[]){(GLMSType){.ast_type = GLMS_AST_TYPE_FUNC}}, 1);

  GIMG *gimg = (GIMG *)ast->ptr;

  GLMSAST *arg0 = args->items[0];

  const char *signature[] = {"uv", "fragCoord", "resolution"};

  GLMSAST *uv_ast = glms_env_new_ast_vec3(eval->env, VEC31(0), false);
  GLMSAST *coord_ast = glms_env_new_ast_vec3(eval->env, VEC31(0), false);
  GLMSAST *res_ast = glms_env_new_ast_vec3(
      eval->env, VEC3(gimg->width, gimg->height, 0), false);

  GLMSAST *signature_values[] = {uv_ast, coord_ast, res_ast};

  GLMSAST *call_ast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_CALL, false);
  call_ast->as.call.func = arg0;

  GLMSStack tmp_stack = {0};
  glms_stack_init(&tmp_stack);
  glms_stack_copy(*stack, &tmp_stack);

  for (int j = 0; j < 3; j++) {
    glms_stack_push(&tmp_stack, signature[j], signature_values[j]);
  }

  for (int x = 0; x < gimg->width; x++) {
    for (int y = 0; y < gimg->height; y++) {
      float u = (float)x / (float)gimg->width;
      float v = (float)y / (float)gimg->height;
      uv_ast->as.v3 = VEC3(u, v, 0);
      coord_ast->as.v3 = VEC3(x, y, 0);

      GLMSAST *result = glms_eval(eval, call_ast, &tmp_stack);

      result = glms_eval(eval, result, &tmp_stack);

      if (result->type == GLMS_AST_TYPE_VEC4) {
        if (!gimg_set_pixel_vec4(gimg, x, y, result->as.v4)) {
	  goto done;
	}
      }
    }
  }

 done:

  glms_stack_clear_trash(&tmp_stack);
  glms_stack_clear(&tmp_stack);

  return ast;
}

const char *glms_struct_image_to_string(GLMSAST *ast) {
  if (!ast)
    return 0;

  if (ast->string_rep != 0) {
    free(ast->string_rep);
    ast->string_rep = 0;
  }

  char *str = 0;

  text_append(&str, "image {\n");

  if (ast->ptr != 0) {
    GIMG *gimg = (GIMG *)ast->ptr;

    {
      char tmp[256];
      sprintf(tmp, "path: %s\n", gimg->uri ? gimg->uri : "?");
      text_append(&str, tmp);
    }

    {
      char tmp[256];
      sprintf(tmp, "width: %d\n", gimg->width);
      text_append(&str, tmp);
    }

    {
      char tmp[256];
      sprintf(tmp, "height: %d\n", gimg->height);
      text_append(&str, tmp);
    }
  }

  text_append(&str, "}\n");

  ast->string_rep = str;

  return ast->string_rep;
}

GLMSAST *glms_struct_image_constructor(GLMSEval *eval, GLMSStack *stack,
                                       GLMSASTList *args, GLMSAST *self) {
  GLMSAST *ast =
      self ? self : glms_env_new_ast(eval->env, GLMS_AST_TYPE_STRUCT, true);
  ast->value_type = ast;
  ast->constructor = glms_struct_image_constructor;
  ast->to_string = glms_struct_image_to_string;

  ast->ptr = ast->ptr ? ast->ptr : NEW(GIMG);
  GIMG *gimg = (GIMG *)ast->ptr;

  if (args != 0 && args->length > 0) {
    GLMSAST *arg0 = glms_eval(eval, args->items[0], stack);

    if (arg0->type == GLMS_AST_TYPE_STRING) {
      gimg_read_from_path(gimg, glms_ast_get_string_value(arg0));
    }
  }

  glms_ast_register_function(eval->env, ast, "getPixel",
                             glms_struct_image_fptr_get_pixel);
  glms_ast_register_function(eval->env, ast, "setPixel",
                             glms_struct_image_fptr_set_pixel);
  glms_ast_register_function(eval->env, ast, "make",
                             glms_struct_image_fptr_make);
  glms_ast_register_function(eval->env, ast, "save",
                             glms_struct_image_fptr_save);
  glms_ast_register_function(eval->env, ast, "shade",
                             glms_struct_image_fptr_shade);

  return ast;
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
  ast->to_string = glms_struct_image_to_string;
  //  ast->ptr = NEW(GIMG);

  glms_env_register_type(env, "image", ast, glms_struct_image_constructor, 0,
                         glms_struct_image_to_string,
                         glms_struct_image_destructor);
}
