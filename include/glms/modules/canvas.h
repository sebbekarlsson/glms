#ifndef GLMS_MODULES_CANVAS_H
#define GLMS_MODULES_CANVAS_H
#include <glms/env.h>
#include <GLFW/glfw3.h>

typedef struct {
  GLFWwindow* win;
  int width;
  int height;

  unsigned int program_id;
  unsigned int vertex_shader_id;
  unsigned int fragment_shader_id;

  unsigned int VAO;
  unsigned int VBO;
  unsigned int EBO;
} GLMSCanvas;

void glms_canvas_type(GLMSEnv *env);

#endif
