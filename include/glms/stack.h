#ifndef GLMS_STACK_H
#define GLMS_STACK_H
#include <hashy/hashy.h>
#include <stdbool.h>
#include <glms/ast.h>

#define GLMS_STACK_CAPACITY 256

typedef struct GLMS_STACK_STRUCT {
  HashyMap locals;
  bool initialized;
  GLMSASTList list;

  const char* names[GLMS_STACK_CAPACITY];
  int names_length;
  int depth;


  bool return_flag;

} GLMSStack;

int glms_stack_init(GLMSStack* stack);

GLMSAST* glms_stack_push(GLMSStack* stack, const char* name, GLMSAST* ast);
GLMSAST* glms_stack_pop(GLMSStack* stack, const char* name);
GLMSAST* glms_stack_get(GLMSStack* stack, const char* name);


int glms_stack_save(GLMSStack* stack);
int glms_stack_restore(GLMSStack* stack);

void glms_stack_dump(GLMSStack* stack);

int glms_stack_copy(GLMSStack src, GLMSStack* dest);

int glms_stack_clear(GLMSStack* stack);
#endif