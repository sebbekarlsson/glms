#ifndef JSCRIPT_STACK_H
#define JSCRIPT_STACK_H
#include <hashy/hashy.h>
#include <stdbool.h>
#include <jscript/ast.h>

#define JSCRIPT_STACK_CAPACITY 256

typedef struct JSCRIPT_STACK_STRUCT {
  HashyMap locals;
  bool initialized;
  JSCRIPTASTList list;

  const char* names[JSCRIPT_STACK_CAPACITY];
  int names_length;
  int depth;


  bool return_flag;

} JSCRIPTStack;

int jscript_stack_init(JSCRIPTStack* stack);

JSCRIPTAST* jscript_stack_push(JSCRIPTStack* stack, const char* name, JSCRIPTAST* ast);
JSCRIPTAST* jscript_stack_pop(JSCRIPTStack* stack, const char* name);
JSCRIPTAST* jscript_stack_get(JSCRIPTStack* stack, const char* name);


int jscript_stack_save(JSCRIPTStack* stack);
int jscript_stack_restore(JSCRIPTStack* stack);

void jscript_stack_dump(JSCRIPTStack* stack);

int jscript_stack_copy(JSCRIPTStack src, JSCRIPTStack* dest);

int jscript_stack_clear(JSCRIPTStack* stack);
#endif
