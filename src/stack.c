#include <glms/env.h>
#include <glms/macros.h>
#include <glms/stack.h>

#include "arena/arena.h"
#include "glms/ast.h"
#include "hashy/hashy.h"

int glms_stack_init(GLMSStack* stack) {
  if (!stack) return 0;
  if (stack->initialized) return 1;
  stack->initialized = true;
  hashy_map_init_v2(&stack->locals,
                    (HashyMapConfig){.remember_keys = true, .capacity = 256});
  stack->return_flag = false;
  stack->depth = 0;

  return 1;
}

GLMSAST* glms_stack_push(GLMSStack* stack, const char* name, GLMSAST* ast) {
  if (!stack || !name || !ast) return 0;
  if (!stack->initialized)
    GLMS_WARNING_RETURN(0, stderr, "stack not initialized.\n");

  GLMSAST* existing = glms_stack_get(stack, name);

  hashy_map_set(&stack->locals, name, ast);

  return ast;
}
GLMSAST* glms_stack_get(GLMSStack* stack, const char* name) {
  if (!stack || !name) return 0;
  if (!stack->initialized)
    GLMS_WARNING_RETURN(0, stderr, "stack not initialized.\n");

  return hashy_map_get(&stack->locals, name);
}
GLMSAST* glms_stack_pop(GLMSStack* stack, const char* name) {
  if (!stack || !name) return 0;
  if (!stack->initialized)
    GLMS_WARNING_RETURN(0, stderr, "stack not initialized.\n");

  GLMSAST* ast = glms_stack_get(stack, name);

  if (!ast) return 0;

  hashy_map_unset(&stack->locals, name);

  return ast;
}

void glms_stack_dump(GLMSStack* stack, GLMSEnv* env) {
  if (!stack) return;
  if (!stack->initialized)
    GLMS_WARNING_RETURN(, stderr, "stack not initialized.\n");

  fprintf(stderr, "not implemented.\n");
}

int glms_stack_copy(GLMSStack src, GLMSStack* dest) {
  if (!dest) return 0;
  if (!dest->initialized) glms_stack_init(dest);

  HashyIterator it = {0};

  while (hashy_map_iterate(&src.locals, &it)) {
    if (!it.bucket->key) continue;
    if (!it.bucket->value) continue;

    const char* key = it.bucket->key;
    GLMSAST* value = (GLMSAST*)it.bucket->value;

    glms_stack_push(dest, key, value);
  }

  return 1;
}

int glms_stack_clear(GLMSStack* stack) {
  if (!stack) return 0;
  if (!stack->initialized)
    GLMS_WARNING_RETURN(0, stderr, "stack not initialized.\n");

  hashy_map_clear(&stack->locals, false);
  return 1;
}

int glms_stack_clear_trash(GLMSStack* stack) {
  if (!stack) return 0;
  if (!stack->initialized)
    GLMS_WARNING_RETURN(0, stderr, "stack not initialized.\n");

  // if (stack->locals.used < 16) {
  //  return 1;
  // }

  HashyIterator it = {0};

  while (hashy_map_iterate(&stack->locals, &it)) {
    if (!it.bucket->key) continue;
    if (!it.bucket->value) continue;

    const char* key = it.bucket->key;
    GLMSAST* value = (GLMSAST*)it.bucket->value;

    if (value->keep) continue;
    if (value->ref.arena == 0) continue;

    hashy_map_unset(&stack->locals, key);
    arena_free(value->ref);
  }

  return 1;
}
