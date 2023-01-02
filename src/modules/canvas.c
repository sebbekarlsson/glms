#include "cglm/struct/affine.h"
#include "cglm/struct/mat4.h"
#include "cglm/types-struct.h"
#include "glms/allocator.h"
#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/env.h"
#include "glms/eval.h"
#include "glms/macros.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glms/modules/canvas.h>

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void glms_canvas_destructor(GLMSAST *ast) {
  if (!ast)
    return;

  if (!ast->ptr)
    return;

  GLMSCanvas* canvas = (GLMSCanvas*)ast->ptr;

  if (!canvas->win) return;

  glDeleteShader(canvas->vertex_shader_id);
  glDeleteShader(canvas->fragment_shader_id);
  // glfwDestroyWindow(canvas->win);
  glDeleteProgram(canvas->program_id);
  canvas->win = 0;

  // free(canvas);
  canvas = 0;
  ast->ptr = 0;
}

char *glms_canvas_to_string(GLMSAST *ast, GLMSAllocator alloc) { return 0; }

int glms_canvas_fptr_shade(GLMSEval *eval, GLMSAST *ast, GLMSASTBuffer *args,
                           GLMSStack *stack, GLMSAST *out) {

  if (!ast->ptr)
    return 0;

  GLMSCanvas *canvas = (GLMSCanvas *)ast->ptr;

  if (!glfwInit()) {
    GLMS_WARNING_RETURN(0, stderr, "Failed to initialize GLFW.\n");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_FLOATING, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  GLFWwindow *window =
      glfwCreateWindow(canvas->width, canvas->height, "canvas", NULL, NULL);

  if (!window) {
    GLMS_WARNING_RETURN(0, stderr, "Failed to create window.\n");
  }

  glfwSetKeyCallback(window, key_callback);

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK) {
    GLMS_WARNING_RETURN(0, stderr, "Failed to initialize GLEW.\n");
  }

  canvas->win = window;

  {
    const char *contents =
        "#version 440\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main() {\n"
        "gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "}";
    int success;
    canvas->vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(canvas->vertex_shader_id, 1, (const GLchar *const *)&(contents), 0);

    glCompileShader(canvas->vertex_shader_id);

    glGetShaderiv(canvas->vertex_shader_id, GL_COMPILE_STATUS, &success);

    if (!success) {
      GLMS_WARNING_RETURN(0, stderr, "failed to compile shader (vertex).\n");
    }
  }

  {
    const char *contents = "#version 440\n"
                           "uniform vec4 color;\n"
                           "out vec4 FragColor;\n"
                           "void main() {\n"
                           "FragColor = color;\n"
                           "}";
    int success;
    canvas->fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(canvas->fragment_shader_id, 1, (const GLchar *const *)&(contents),
                   0);

    glCompileShader(canvas->fragment_shader_id);

    glGetShaderiv(canvas->fragment_shader_id, GL_COMPILE_STATUS, &success);

    if (!success) {
      GLMS_WARNING_RETURN(0, stderr, "failed to compile shader (fragment).\n");
    }
  }

  canvas->program_id = glCreateProgram();

  {
    int success = 1;
    glAttachShader(canvas->program_id, canvas->vertex_shader_id);
    glLinkProgram(canvas->program_id);
    glGetProgramiv(canvas->program_id, GL_LINK_STATUS, &success);

    if (!success) {
      GLMS_WARNING_RETURN(0, stderr, "failed to link program (vertex).\n");
    }
  }

  {
    int success = 1;
    glAttachShader(canvas->program_id, canvas->fragment_shader_id);
    glLinkProgram(canvas->program_id);
    glGetProgramiv(canvas->program_id, GL_LINK_STATUS, &success);

    if (!success) {
      GLMS_WARNING_RETURN(0, stderr, "failed to link program (fragment).\n");
    }
  }

  GLMSAST arg0 = args->items[0];

  const char *signature[] = {"uv", "fragCoord", "resolution", "time"};

  GLMSAST *uv_ast = glms_env_new_ast_vec3(eval->env, VEC31(0), false);
  GLMSAST *coord_ast = glms_env_new_ast_vec3(eval->env, VEC31(0), false);
  GLMSAST *res_ast = glms_env_new_ast_vec3(
      eval->env, VEC3(canvas->width, canvas->height, 0), false);
  GLMSAST *time_ast = glms_env_new_ast_number(eval->env, 0, false);

  GLMSAST *signature_values[] = {uv_ast, coord_ast, res_ast, time_ast};

  GLMSAST call_ast =
      (GLMSAST){.type = GLMS_AST_TYPE_CALL}; // glms_env_new_ast(eval->env,
                                             // GLMS_AST_TYPE_CALL, false);
  call_ast.as.call.func = &arg0;

  for (int j = 0; j < 4; j++) {
    glms_stack_push(stack, signature[j], signature_values[j]);
  }

  glUseProgram(canvas->program_id);

  mat4s projection = glms_ortho(0, (float)canvas->width, 0,
                                (float)canvas->height, -100.0f, 400.0f);
  mat4s view = glms_mat4_identity();
  mat4s model = glms_mat4_identity();
  glUniformMatrix4fv(glGetUniformLocation(canvas->program_id, "projection"), 1,
                     GL_FALSE, *projection.raw);

  glUniformMatrix4fv(glGetUniformLocation(canvas->program_id, "view"), 1,
                     GL_FALSE, *view.raw);

  float w = 16.0f;
  float h = 16.0f;

  glGenVertexArrays(1, &canvas->VAO);
  glGenBuffers(1, &canvas->VBO);
  glGenBuffers(1, &canvas->EBO);

  glBindVertexArray(canvas->VAO);

  Vector3 *vertices = (Vector3[]){VEC3(w * 0, h * 1, 0), VEC3(w * 1, h * 1, 0),
                                  VEC3(w * 1, h * 0, 0), VEC3(w * 0, h * 0, 0)};

  unsigned int *indices = (unsigned int[]){0, 1, 3, 1, 2, 3};

  glBindBuffer(GL_ARRAY_BUFFER, canvas->VBO);
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vector3), &vertices[0],
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, canvas->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), &indices[0],
               GL_STATIC_DRAW);

  int vertex_loc = 0;
  glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3),
                        (void *)0);
  glEnableVertexAttribArray(vertex_loc);

  while (!glfwWindowShouldClose(window)) {
    //    glfwSwapInterval(1);
    glfwPollEvents();
    glViewport(0, 0, canvas->width, canvas->height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    time_ast->as.number.value = (float)glfwGetTime();
    for (int x = 0; x < canvas->width; x += w) {
      for (int y = 0; y < canvas->height; y += h) {
        float u = (float)x / (float)canvas->width;
        float v = (float)y / (float)canvas->height;
        uv_ast->as.v3 = VEC3(u, v, 0);
        coord_ast->as.v3 = VEC3(x, y, 0);

        GLMSAST result = glms_eval(eval, call_ast, stack);

        result = glms_eval(eval, result, stack);

        if (result.type == GLMS_AST_TYPE_VEC4) {
          Vector4 value = result.as.v4;

          glUniform4f(glGetUniformLocation(canvas->program_id, "color"),
                      value.x, value.y, value.z, value.w);

          model = glms_mat4_identity();
          model = glms_translate(model, (vec3s){x, y, 0});
          glUniformMatrix4fv(glGetUniformLocation(canvas->program_id, "model"),
                             1, GL_FALSE, *model.raw);

          glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
      }
    }

    glfwSwapBuffers(window);
  }

  return 1;
}

void glms_canvas_constructor(GLMSEval *eval, GLMSStack *stack,
                             GLMSASTBuffer *args, GLMSAST *self) {

  if (!self)
    return;
  self->type = GLMS_AST_TYPE_STRUCT;
  self->constructor = glms_canvas_constructor;
  self->destructor = glms_canvas_destructor;
  if (self->ptr)
    return;

  if (!args || args->length <= 0)
    return;
  if (!glms_eval_expect(
          eval, stack,
          (GLMSASTType[]){GLMS_AST_TYPE_NUMBER, GLMS_AST_TYPE_NUMBER}, 2, args))
    return;

  float width = glms_ast_number(args->items[0]);
  float height = glms_ast_number(args->items[1]);

  GLMSCanvas *canvas = NEW(GLMSCanvas);
  canvas->width = (int)width;
  canvas->height = (int)height;
  self->ptr = canvas;
  // self->to_string = glms_canvas_to_string;
  glms_ast_register_function(eval->env, self, "shade", glms_canvas_fptr_shade);

  glms_env_register_function_signature(
      eval->env, self, "shade",
      (GLMSFunctionSignature){.return_type = (GLMSType){GLMS_AST_TYPE_BOOL},
                              .args =
                                  (GLMSType[]){(GLMSType){GLMS_AST_TYPE_FUNC}},
                              .args_length = 1});
}

void glms_canvas_type(GLMSEnv *env) {
  glms_env_register_type(env, "canvas",
                         glms_env_new_ast(env, GLMS_AST_TYPE_STRUCT, false),
                         glms_canvas_constructor, 0, 0, glms_canvas_destructor);
}
