#include "arena/arena.h"
#include "hashy/hashy.h"
#include <glms/macros.h>
#include <glms/stack.h>

int glms_stack_init(GLMSStack *stack) {
  if (!stack)
    return 0;
  if (stack->initialized)
    return 1;
  stack->initialized = true;
  hashy_map_init_v2(&stack->locals,
                    (HashyMapConfig){.remember_keys = true, .capacity = 256});
  glms_GLMSAST_list_init(&stack->list);
  stack->names_length = 0;
  stack->return_flag = false;
  stack->depth = 0;

  return 1;
}

GLMSAST *glms_stack_push(GLMSStack *stack, const char *name, GLMSAST *ast) {
  if (!stack || !name || !ast)
    return 0;
  if (!stack->initialized)
    GLMS_WARNING_RETURN(0, stderr, "stack not initialized.\n");

  if (stack->names_length >= GLMS_STACK_CAPACITY) {
    GLMS_WARNING(stderr, "Stack Stackoverflow: %d\n", stack->names_length);
    glms_stack_dump(stack);
    return 0;
  }

  GLMSAST* existing = glms_stack_get(stack, name);

  if (!existing) {
    stack->names[stack->names_length++] = name;
    glms_GLMSAST_list_push(&stack->list, ast);
  }

 hashy_map_set(&stack->locals, name, ast);
  


  return ast;
}
GLMSAST *glms_stack_get(GLMSStack *stack, const char *name) {
  if (!stack || !name)
    return 0;
  if (!stack->initialized)
    GLMS_WARNING_RETURN(0, stderr, "stack not initialized.\n");

  return hashy_map_get(&stack->locals, name);
}
GLMSAST *glms_stack_pop(GLMSStack *stack, const char *name) {
  if (!stack || !name)
    return 0;
  if (!stack->initialized)
    GLMS_WARNING_RETURN(0, stderr, "stack not initialized.\n");

  GLMSAST *ast = glms_stack_get(stack, name);

  if (!ast)
    return 0;

  hashy_map_unset(&stack->locals, name);

  glms_GLMSAST_list_remove(&stack->list, ast);

  stack->names[stack->names_length--] = 0;

  return ast;
}

void glms_stack_dump(GLMSStack *stack) {
  if (!stack)
    return;
  if (!stack->initialized)
    GLMS_WARNING_RETURN(, stderr, "stack not initialized.\n");

  for (int i = 0; i < stack->names_length; i++) {
    GLMSAST *ast = glms_stack_get(stack, stack->names[i]);

    printf("%d (%s) => %s\n", i, stack->names[i],
           stack->names[i]);
  }
}

int glms_stack_copy(GLMSStack src, GLMSStack *dest) {
  if (!dest)
    return 0;
  if (!dest->initialized)
    glms_stack_init(dest);

  for (int i = 0; i < src.names_length; i++) {
    GLMSAST *ast = glms_stack_get(&src, src.names[i]);
    if (!ast)
      continue;

    glms_stack_push(dest, src.names[i], ast);
  }

  if (src.names_length > 0) {
    dest->names_length = src.names_length;
    memcpy(&dest->names[0], &src.names[0], GLMS_STACK_CAPACITY * sizeof(char));
  }

  return 1;
}

int glms_stack_clear(GLMSStack *stack) {
  if (!stack)
    return 0;
  if (!stack->initialized)
    GLMS_WARNING_RETURN(0, stderr, "stack not initialized.\n");

  hashy_map_clear(&stack->locals, false);
  glms_GLMSAST_list_clear(&stack->list);
  stack->names_length = 0;
  return 1;
}

int glms_stack_clear_trash(GLMSStack *stack) {
  if (!stack)
    return 0;
  if (!stack->initialized)
    GLMS_WARNING_RETURN(0, stderr, "stack not initialized.\n");


  // if (stack->locals.used < 16) {
  //  return 1;
  // }

HashyIterator it = {0};

  while (hashy_map_iterate(&stack->locals, &it)) {
    if (!it.bucket->key)
      continue;
    if (!it.bucket->value)
      continue;

    const char *key = it.bucket->key;
    GLMSAST *value = (GLMSAST *)it.bucket->value;

    if (value->keep) continue;
    if (value->ref.arena == 0) continue;

    hashy_map_unset(&stack->locals, key);
    arena_free(value->ref);
  }

  return 1;
}
