#include <jscript/stack.h>
#include <jscript/macros.h>



int jscript_stack_init(JSCRIPTStack* stack) {
  if (!stack) return 0;
  if (stack->initialized) return 1;
  stack->initialized = true;
  hashy_map_init_v2(&stack->locals, (HashyMapConfig){ .remember_keys = true, .capacity = 256 });
  jscript_JSCRIPTAST_list_init(&stack->list);
  stack->names_length = 0;
  stack->return_flag = false;
  stack->depth = 0;

  return 1;
}

JSCRIPTAST* jscript_stack_push(JSCRIPTStack* stack, const char* name, JSCRIPTAST* ast) {
  if (!stack || !name || !ast) return 0;
  if (!stack->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "stack not initialized.\n");

  if (stack->names_length >= JSCRIPT_STACK_CAPACITY) JSCRIPT_WARNING_RETURN(0, stderr, "Stackoverflow\n");
  hashy_map_set(&stack->locals, name, ast);
  jscript_JSCRIPTAST_list_push(&stack->list, ast);

  stack->names[stack->names_length++] = name;

  return ast;
}
JSCRIPTAST* jscript_stack_get(JSCRIPTStack* stack, const char* name) {
  if (!stack || !name) return 0;
  if (!stack->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "stack not initialized.\n");

  return hashy_map_get(&stack->locals, name);
}
JSCRIPTAST* jscript_stack_pop(JSCRIPTStack* stack, const char* name) {
  if (!stack || !name) return 0;
  if (!stack->initialized) JSCRIPT_WARNING_RETURN(0, stderr, "stack not initialized.\n");

  JSCRIPTAST* ast = jscript_stack_get(stack, name);

  if (!ast) return 0;

  hashy_map_unset(&stack->locals, name);

  jscript_JSCRIPTAST_list_remove(&stack->list, ast);

  stack->names[stack->names_length--] = 0;

  return ast;
}

void jscript_stack_dump(JSCRIPTStack* stack) {
  if (!stack) return;
  if (!stack->initialized) JSCRIPT_WARNING_RETURN(,stderr, "stack not initialized.\n");


  for (int i = 0; i < stack->names_length; i++) {
    printf("%d => %s\n", i, stack->names[i]);
  }
}

int jscript_stack_copy(JSCRIPTStack src, JSCRIPTStack* dest) {
  if (!dest) return 0;
  if (!dest->initialized) jscript_stack_init(dest);


  for (int i = 0; i < src.names_length; i++) {
    JSCRIPTAST* ast = jscript_stack_get(&src, src.names[i]);
    if (!ast) continue;

    jscript_stack_push(dest, src.names[i], ast);
  }

  return 1;
}

int jscript_stack_clear(JSCRIPTStack* stack) {
  if (!stack) return 0;
  if (!stack->initialized) JSCRIPT_WARNING_RETURN(0,stderr, "stack not initialized.\n");


  hashy_map_clear(&stack->locals, false);
  jscript_JSCRIPTAST_list_clear(&stack->list);
  stack->names_length = 0;
  return 1;
}
