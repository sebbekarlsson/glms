#include <glms/builtin.h>
#include <glms/dl.h>
#include <glms/env.h>
#include <glms/eval.h>
#include <glms/io.h>
#include <glms/macros.h>
#include <string.h>
#include <text/text.h>

#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/fptr.h"
#include "glms/stack.h"
#include "glms/string_view.h"
#include "glms/token.h"
#include "hashy/hashy.h"
#include "hashy/keylist.h"

#define GLMS_AST_DEBUG_PRINT(ast)                                              \
  { printf("%s\n", glms_ast_to_string(ast, eval->env->string_alloc)); }

GLMSAST *glms_eval_get_type(GLMSEval *eval, GLMSStack *stack, GLMSAST *ast) {
  if (!eval || !ast)
    return 0;

  GLMSAST *ptr = glms_ast_get_ptr(*ast);

  if (ptr)
    return glms_eval_get_type(eval, stack, ptr);

  GLMSAST *t = ast->value_type;
  GLMSAST *t2 = 0;

  if (t != 0) {
    const char *key = glms_ast_get_string_value(t);
    t2 = glms_env_lookup_type(eval->env, key);
  }

  if (t != 0 && t->constructor)
    return t;
  if (t2 != 0 && t2->constructor)
    return t2;

  const char *key = glms_ast_get_string_value(ast);

  GLMSAST *t3 = glms_env_lookup_type(eval->env, key);

  if (t3 != 0 && t3->constructor)
    return t3;

  return t ? t : t2 ? t2 : t3 ? t3 : 0;
}

int glms_eval_init(GLMSEval *eval, struct GLMS_ENV_STRUCT *env) {
  if (!eval || !env)
    return 0;
  if (eval->initialized)
    return 1;
  eval->initialized = true;
  eval->env = env;
  return 1;
}

GLMSAST *glms_eval_lookup(GLMSEval *eval, GLMSStack *stack, const char *key) {
  if (!key || !eval || !stack)
    return 0;

  GLMSAST *t = glms_env_lookup_type(eval->env, key);

  if (t) {
    if (t->constructor && t->constructed == false) {
      t->constructor(eval, stack, 0, t);
      t->constructed = true;
    }
    return t;
  }

  GLMSAST *global = (GLMSAST *)hashy_map_get(&eval->env->globals, key);
  if (global)
    return global;

  return glms_stack_get(stack, key);
}

int glms_eval_construct_string(GLMSEval *eval, GLMSAST *ptr, GLMSStack *stack) {

  GLMSAST* astptr = glms_ast_get_ptr(*ptr);
  if (astptr != 0) ptr = astptr;
  
  if (ptr->type != GLMS_AST_TYPE_STRING)
    return 0;

  if (ptr->children == 0 || ptr->children->length <= 0)
    return 0;

  char *str = 0;

  for (int64_t i = 0; i < ptr->children->length; i++) {
    GLMSAST evaluated = glms_eval(eval, *ptr->children->items[i], stack);
    GLMSAST* eval_ptr = glms_ast_get_ptr(evaluated);
    const char *childstr = glms_ast_get_string_value(eval_ptr ? eval_ptr : &evaluated);
    if (!childstr) {
      childstr = "?";
    }

    text_append(&str, childstr);
  }

  if (str == 0) return 0;

  if (ptr->as.string.heap != 0) {
    free(ptr->as.string.heap);
    ptr->as.string.heap = 0;
  }

  ptr->as.string.heap = str;

  glms_GLMSAST_list_clear(ptr->children);
  ptr->children = 0;

  return 1;
}

void glms_eval_push_args(GLMSEval *eval, GLMSStack *stack, GLMSAST *func,
			 GLMSASTBuffer args) {
  if (args.length <= 0 || args.items == 0)
    return;

  if (!func->children || func->children->length <= 0)
    return;

  int64_t n = MIN(func->children->length, args.length);

  for (int64_t i = 0; i < n; i++) {
    GLMSAST *arg = func->children->items[i];
    const char *name = glms_ast_get_name(arg);
    if (!name)
      name = glms_ast_get_name(&args.items[i]);

    glms_stack_push(stack, name, glms_ast_copy(args.items[i], eval->env));
  }
}

GLMSAST glms_eval_call_func(GLMSEval *eval, GLMSStack *stack, GLMSAST *func,
			    GLMSASTBuffer args) {
  GLMSFPTR fptr = func->fptr;

  GLMSAST *self = glms_stack_get(stack, "self");
  const char *fname = glms_ast_get_name(func);

  if (self == 0) {
    self = func;
    glms_stack_push(stack, "self", self);
  }

  if (func->constructor) {
    GLMSAST *new_ast =
	glms_env_new_ast(eval->env, GLMS_AST_TYPE_UNDEFINED, true);
    func->constructor(eval, stack, &args, new_ast);
    new_ast->constructed = true;
    return *new_ast;
  }

  // constructor
  if (func->type == GLMS_AST_TYPE_STRUCT) {
    GLMSAST *copied = glms_ast_copy(*func, eval->env);

    GLMSStack tmp_stack = {0};
    glms_stack_init(&tmp_stack);
    glms_stack_copy(*stack, &tmp_stack);

    if (args.length > 0 && func->props.initialized) {
      HashyIterator it = {0};

      int64_t i = 0;
      while (hashy_map_iterate(&func->props, &it)) {
	if (!it.bucket->key)
	  continue;
	if (!it.bucket->value)
	  continue;

	const char *key = it.bucket->key;
	GLMSAST *val = (GLMSAST *)it.bucket->value;
	GLMSAST value = glms_eval(eval, *val, &tmp_stack);

	GLMSAST arg_value = glms_eval(eval, args.items[i], &tmp_stack);
	glms_ast_object_set_property(copied, key,
				     glms_ast_copy(arg_value, eval->env));
	i++;

	if (i >= args.length) {
	  hashy_key_list_clear(&it.keys);
	  break;
	}
      }
    }

    glms_stack_clear(&tmp_stack);

    GLMSAST ptr =
	(GLMSAST){.type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = copied};
    return ptr;
  }

  if (fptr) {
    GLMSStack tmp_stack = {0};
    glms_stack_init(&tmp_stack);
    glms_stack_copy(*stack, &tmp_stack);

    glms_eval_push_args(eval, &tmp_stack, func, args);

    GLMSAST result = {0};
    if (fptr(eval, self, &args, &tmp_stack, &result)) {
      if (result.type == GLMS_AST_TYPE_STACK_PTR) {
	glms_env_apply_type(eval->env, eval, stack, result.as.stackptr.ptr);
      } else {
#if 0
	GLMSAST* new_ast = glms_ast_copy(result, eval->env);
	glms_env_apply_type(eval->env, eval, stack, new_ast);
	GLMSAST stackptr = (GLMSAST){ .type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_ast };
	result = stackptr;
#endif
      }
      glms_stack_clear(&tmp_stack);
      return glms_eval(eval, result, stack);
    }
    glms_stack_clear(&tmp_stack);
  }

  if (func->as.func.body != 0) {
    GLMSStack tmp_stack = {0};
    glms_stack_init(&tmp_stack);
    glms_stack_copy(*stack, &tmp_stack);

    if (func->children != 0) {
      for (int64_t i = 0; i < MIN(args.length, func->children->length); i++) {
	GLMSAST arg_value = glms_eval(eval, args.items[i], &tmp_stack);
	GLMSAST *arg_func = func->children->items[i];

	const char *arg_name = glms_ast_get_name(arg_func);
	if (!arg_name)
	  continue;

	GLMSAST *copy = glms_ast_copy(arg_value, eval->env);

	glms_stack_push(&tmp_stack, arg_name, copy);
      }
    }

    GLMSAST result = glms_eval(eval, *func->as.func.body, &tmp_stack);
    glms_stack_clear(&tmp_stack);
    return result;
  }

  return *func;
}

GLMSAST glms_eval_call(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  GLMSAST *func = ast.as.call.func;
  const char *name = glms_string_view_get_value(&ast.as.func.id->as.id.value);

  if (!func && name != 0) {
    func = glms_eval_lookup(eval, stack, name);
  }

  if (func && func->type == GLMS_AST_TYPE_STACK_PTR) {
    GLMSAST *ptr = glms_ast_get_ptr(*func);
    if (ptr != 0)
      func = ptr;
  }

  GLMSASTBuffer args = {0};
  glms_GLMSAST_buffer_init(&args);

  GLMSFPTR overload = 0;

  if (ast.children != 0) {
    for (int64_t i = 0; i < ast.children->length; i++) {
      GLMSAST arg = glms_eval(eval, *ast.children->items[i], stack);

      GLMSAST *ptr = 0;
      if ((ptr = glms_ast_get_ptr(arg))) {
	glms_env_apply_type(eval->env, eval, stack, ptr);
	arg = *ptr;
      }

      // TODO: this is to have certain types describe that they contain elements
      // of which operations should be applied upon instead of the actual type.
      // Not sure if I want to do it this way yet.

      // GLMSASTBuffer atoms = {0};
      //      if (glms_ast_get_atoms(arg, &atoms)) {
      //	for (int64_t j = 0; j < atoms.length; j++) {
      //	  GLMSAST atom = atoms.items[j];
      //	  glms_GLMSAST_buffer_push(&args, atom);
      //	}
      //	glms_GLMSAST_buffer_clear(&atoms);
      //      } else {

      overload = overload ? overload : glms_ast_get_func_overload(arg, name);
      glms_GLMSAST_buffer_push(&args, arg);
      // }
    }
  }

  if (!func) {
    GLMS_WARNING_RETURN(ast, stderr, "No such function `%s`\n", name);
  }

  GLMSAST result = {0};

  if (overload != 0) {
    GLMSAST tmp_func = (GLMSAST){.type = GLMS_AST_TYPE_FUNC, .fptr = overload};
    result = glms_eval_call_func(eval, stack, &tmp_func, args);
  } else {
    result = glms_eval_call_func(eval, stack, func, args);
  }

  glms_GLMSAST_buffer_clear(&args);
  return result;
}

GLMSAST glms_eval_compound(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  if (ast.children == 0 || ast.children->length <= 0)
    return ast;

  for (int64_t i = 0; i < ast.children->length; i++) {
    GLMSAST *child = ast.children->items[i];

    GLMSAST evaluated = glms_eval(eval, *child, stack);

    if (evaluated.type == GLMS_AST_TYPE_UNOP &&
	evaluated.as.unop.op == GLMS_TOKEN_TYPE_SPECIAL_BREAK) {
      return evaluated;
    }

    if (stack->return_flag) {
      stack->return_flag = false;
      GLMSAST *retval = glms_eval_lookup(eval, stack, "return");

      if (retval) {
	return glms_eval(eval, *retval, stack);
      }
    }
  }

  return ast;
}

GLMSAST glms_eval_assign(GLMSEval *eval, GLMSAST left, GLMSAST right,
			 GLMSStack *stack) {
  const char *name = 0;

  // if (left.type == GLMS_AST_TYPE_ID) {
  name = glms_ast_get_name(
      &left); // glms_string_view_get_value(&left.as.id.value);
  //}

  GLMSAST *existing = glms_ast_get_ptr(left);
  GLMSAST *ptr = glms_ast_get_ptr(right);

  if (existing == 0 && name != 0) {
    existing = glms_eval_lookup(eval, stack, name);
  }

  if (existing) {
    glms_ast_assign(existing, ptr ? (*ptr) : right, eval, stack);
  } else if (name) {
    GLMSAST *copy = ptr ? ptr : glms_ast_copy(right, eval->env);
    GLMSAST t = {0};
    if (copy->constructed == false && glms_ast_get_type(left, &t)) {
      GLMSAST *look =
	  glms_env_lookup_type(copy->env_ref ? copy->env_ref : eval->env,
			       glms_string_view_get_value(&t.as.id.value));

      if (look && look->constructor) {
	look->constructor(eval, stack, 0, copy);
	copy->constructed = true;
	copy->value_type = look;
      }
    }

    glms_stack_push(stack, name, copy);
  }

  return right;
}

GLMSAST glms_eval_id(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  const char *name = glms_string_view_get_value(&ast.as.id.value);
  GLMSAST *value = 0;

  value = ast.env_ref ? glms_env_lookup(ast.env_ref, name) : 0;
  value = value ? value : glms_eval_lookup(eval, stack, name);

  if (value != 0) {
    glms_env_apply_type(ast.env_ref ? ast.env_ref : eval->env, eval, stack,
			value);
    return (GLMSAST){.type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = value};
  } else if (value == 0 && ((ast.flags == 0) || (ast.flags->length <= 0))) {
    GLMS_WARNING_RETURN((GLMSAST){.type = GLMS_AST_TYPE_UNDEFINED}, stderr,
			"`%s` is not defined.", name);
  }

  return ast;
}

GLMSAST glms_eval_stack_ptr(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  GLMSAST *ptr = glms_ast_get_ptr(ast);

  if (ptr) {
    glms_env_apply_type(eval->env, eval, stack, ptr);

    glms_eval_construct_string(eval, ptr, stack);
  }

  return ast;
}

GLMSAST glms_eval_unop_left(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  switch (ast.as.unop.op) {
  case GLMS_TOKEN_TYPE_SUB: {
    GLMSAST left = glms_eval(eval, *ast.as.unop.left, stack);
    return (GLMSAST){.type = GLMS_AST_TYPE_NUMBER,
		     .as.number = -left.as.number.value};
  }; break;
  case GLMS_TOKEN_TYPE_ADD: {
    GLMSAST left = glms_eval(eval, *ast.as.unop.left, stack);
    return (GLMSAST){.type = GLMS_AST_TYPE_NUMBER,
		     .as.number = +left.as.number.value};
  }; break;
  case GLMS_TOKEN_TYPE_ADD_ADD: {
    GLMSAST left = glms_eval(eval, *ast.as.unop.left, stack);
    return glms_ast_op_add_add(&left);
  }; break;
  case GLMS_TOKEN_TYPE_SUB_SUB: {
    GLMSAST left = glms_eval(eval, *ast.as.unop.left, stack);
    return glms_ast_op_sub_sub(&left);
  }; break;
  default: {
    return ast;
  }; break;
  }
  return ast;
}

GLMSAST glms_eval_unop_right(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  switch (ast.as.unop.op) {
  case GLMS_TOKEN_TYPE_SUB: {
    GLMSAST right = glms_eval(eval, *ast.as.unop.right, stack);
    return (GLMSAST){.type = GLMS_AST_TYPE_NUMBER,
		     .as.number = -right.as.number.value};
  }; break;
  case GLMS_TOKEN_TYPE_ADD: {
    GLMSAST right = glms_eval(eval, *ast.as.unop.right, stack);
    return (GLMSAST){.type = GLMS_AST_TYPE_NUMBER,
		     .as.number = +right.as.number.value};
  }; break;
  case GLMS_TOKEN_TYPE_EXCLAM: {
    GLMSAST right = glms_eval(eval, *ast.as.unop.right, stack);
    return (GLMSAST){.type = GLMS_AST_TYPE_BOOL,
		     .as.boolean = !glms_ast_is_truthy(right)};
  }; break;
  case GLMS_TOKEN_TYPE_ADD_ADD: {
    GLMSAST right = glms_eval(eval, *ast.as.unop.right, stack);
    return glms_ast_op_add_add(&right);
  }; break;
  case GLMS_TOKEN_TYPE_SUB_SUB: {
    GLMSAST right = glms_eval(eval, *ast.as.unop.right, stack);
    return glms_ast_op_sub_sub(&right);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_RETURN: {
    GLMSAST right = glms_eval(eval, *ast.as.unop.right, stack);

    GLMSAST *retval = glms_ast_copy(right, eval->env);
    glms_env_apply_type(eval->env, eval, stack, retval);

    glms_stack_push(stack, "return", retval);
    stack->return_flag = true;
    return right;
  }; break;
  default: {
    return ast;
  }; break;
  }
  return ast;
}

GLMSAST glms_eval_unop(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  if (ast.as.unop.left)
    return glms_eval_unop_left(eval, ast, stack);
  return glms_eval_unop_right(eval, ast, stack);
}

GLMSAST glms_eval_binop(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  // bool is_assign = ast.as.binop.op == GLMS_TOKEN_TYPE_EQUALS;
  GLMSAST left = glms_eval(eval, *ast.as.binop.left, stack);
  GLMSAST right = glms_eval(eval, *ast.as.binop.right, stack);

  GLMSAST *ptr_left = 0;
  GLMSAST *ptr_right = 0;

  GLMSAST l = left;
  GLMSAST r = right;

  if ((ptr_left = glms_ast_get_ptr(left))) {
    glms_env_apply_type(eval->env, eval, stack, ptr_left);
    l = *ptr_left;
  }
  if ((ptr_right = glms_ast_get_ptr(right))) {
    glms_env_apply_type(eval->env, eval, stack, ptr_right);
    r = *ptr_right;
  }

  GLMSASTOperatorOverload overload =
      glms_ast_get_op_overload(l, ast.as.binop.op, eval->env);

  if (!overload)
    overload = glms_ast_get_op_overload(r, ast.as.binop.op, eval->env);

  if (overload != 0) {
    GLMSAST result = {0};

    if (overload(eval, stack, &l, &r, &result)) {
      return glms_eval(eval, result, stack);
    }
  }

  switch (ast.as.binop.op) {
  case GLMS_TOKEN_TYPE_EQUALS_EQUALS: {
    return glms_ast_op_eq(left, right);
  }; break;
  case GLMS_TOKEN_TYPE_ADD_EQUALS: {
    return glms_ast_op_add_eq(&left, right);
  }; break;
  case GLMS_TOKEN_TYPE_SUB_EQUALS: {
    return glms_ast_op_sub_eq(&left, right);
  }; break;
  case GLMS_TOKEN_TYPE_MUL_EQUALS: {
    return glms_ast_op_mul_eq(&left, right);
  }; break;
  case GLMS_TOKEN_TYPE_DIV_EQUALS: {
    return glms_ast_op_div_eq(&left, right);
  }; break;
  case GLMS_TOKEN_TYPE_AND_AND: {
    return glms_ast_op_and_and(left, right);
  }; break;
  case GLMS_TOKEN_TYPE_PIPE_PIPE: {
    return glms_ast_op_pipe_pipe(left, right);
  }; break;
  case GLMS_TOKEN_TYPE_LT: {
    return glms_ast_op_lt(left, right);
  }; break;
  case GLMS_TOKEN_TYPE_LTE: {
    return glms_ast_op_lte(left, right);
  }; break;
  case GLMS_TOKEN_TYPE_GT: {
    return glms_ast_op_gt(left, right);
  }; break;
  case GLMS_TOKEN_TYPE_GTE: {
    return glms_ast_op_gte(left, right);
  }; break;
  case GLMS_TOKEN_TYPE_MUL: {
    return glms_ast_op_mul(left, right);
  }; break;
  case GLMS_TOKEN_TYPE_DIV: {
    return glms_ast_op_div(left, right);
  }; break;
  case GLMS_TOKEN_TYPE_SUB: {
    return glms_ast_op_sub(left, right);
  }; break;
  case GLMS_TOKEN_TYPE_ADD: {
    return glms_ast_op_add(left, right);
  }; break;
  case GLMS_TOKEN_TYPE_PERCENT: {
    return glms_ast_op_mod(left, right);
  }; break;
  case GLMS_TOKEN_TYPE_EQUALS: {
    return glms_eval_assign(eval, left, right, stack);
  }; break;
  default: {
    return ast;
  }; break;
  }

  return ast;
}

GLMSAST glms_eval_for(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  if (!ast.as.forloop.body) {
    return ast;
  }
  if (ast.children == 0 || ast.children->length <= 0) {
    return ast;
  }

  JAST init = *ast.children->items[0];
  JAST cond = *ast.children->items[1];
  JAST step = *ast.children->items[2];

  for (glms_eval(eval, init, stack);
       glms_ast_is_truthy(glms_eval(eval, cond, stack));
       glms_eval(eval, step, stack)) {
    glms_eval(eval, *ast.as.forloop.body, stack);
  }

  return ast;
}

GLMSAST glms_eval_block_condition(GLMSEval *eval, GLMSAST ast,
				  GLMSStack *stack) {
  if (ast.as.block.expr) {
    GLMSAST expr = glms_eval(eval, *ast.as.block.expr, stack);

    if (ast.as.block.body && glms_ast_is_truthy(expr)) {
      return glms_eval(eval, *ast.as.block.body, stack);
    } else {
      if (ast.as.block.next) {
	return glms_eval(eval, *ast.as.block.next, stack);
      }
      return ast;
    }
  }

  if (ast.as.block.next) {
    return glms_eval(eval, *ast.as.block.next, stack);
  }

  return glms_eval(eval, *ast.as.block.body, stack);
}

GLMSAST glms_eval_block_while(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  if (!ast.as.block.body || !ast.as.block.expr)
    return ast;

  GLMSAST *ptr = glms_ast_get_ptr(*ast.as.block.expr);
  if (!ptr)
    ptr = ast.as.block.expr;

  GLMSAST evaluated = *ptr;
  //  while (glms_ast_is_truthy((evaluated = glms_eval(eval, evaluated,
  //  stack)))) { glms_eval(eval, *ast.as.block.body, stack);

  //}

  while (glms_ast_is_truthy((glms_eval(eval, *ast.as.block.expr, stack)))) {
    GLMSAST result = glms_eval(eval, *ast.as.block.body, stack);

    if (result.type == GLMS_AST_TYPE_UNOP &&
	result.as.unop.op == GLMS_TOKEN_TYPE_SPECIAL_BREAK) {
      break;
    }
  }

  return ast;
}

GLMSAST glms_eval_block_switch(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  if (!ast.as.block.body || !ast.as.block.expr)
    return ast;

  GLMSAST *body = ast.as.block.body;

  if (!body->children || body->children->length <= 0)
    return ast;

  GLMSAST expr = glms_eval(eval, *ast.as.block.expr, stack);

  for (int64_t i = 0; i < body->children->length; i++) {
    GLMSAST *child = body->children->items[i];
    if (child->type != GLMS_AST_TYPE_BLOCK)
      GLMS_WARNING_RETURN(ast, stderr, "Invalid switch body item.\n");
    if (!child->as.block.expr || !child->as.block.body)
      continue;

    GLMSAST child_expr = glms_eval(eval, *child->as.block.expr, stack);

    if (glms_ast_compare_equals_equals(expr, child_expr)) {
      return glms_eval(eval, *child->as.block.body, stack);
    }
  }

  return ast;
}

GLMSAST glms_eval_block(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  switch (ast.as.block.op) {
  case GLMS_TOKEN_TYPE_SPECIAL_WHILE: {
    return glms_eval_block_while(eval, ast, stack);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_IF: {
    return glms_eval_block_condition(eval, ast, stack);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_ELSE: {
    return glms_eval_block_condition(eval, ast, stack);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_SWITCH: {
    return glms_eval_block_switch(eval, ast, stack);
  }; break;
  default: {
    return ast;
  }; break;
  }

  return ast;
}

GLMSAST glms_eval_access_by_key(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  GLMSAST left = glms_eval(eval, *ast.as.access.left, stack);
  GLMSAST right = *ast.as.access.right;

  GLMSAST *ptr = glms_ast_get_ptr(left);

  GLMSAST *L = ptr ? ptr : &left;

  if (L->swizzle && right.type == GLMS_AST_TYPE_ID) {
    GLMSAST sw = {0};
    if (L->swizzle(eval, stack, L, &right, &sw)) {
      return glms_eval(eval, sw, stack);
    }
  }

  const char *key = glms_ast_get_string_value(&right);

  GLMSAST *value = glms_ast_access_by_key(L, key, eval->env);

  GLMSAST *vptr = value ? glms_ast_get_ptr(*value) : 0;

  if (vptr)
    value = vptr;

  // GLMSAST *t = glms_eval_get_type(eval, stack, L);

  //  if (t != 0) {
  //  value = value ? value : glms_ast_access_by_key(t, key, eval->env);
  // }

  if (value) {
    if (value->type == GLMS_AST_TYPE_FUNC && right.type == GLMS_AST_TYPE_CALL) {
      right.as.call.func = value;

      if (ptr != 0) {
	glms_stack_push(stack, "self", ptr);
      }
      return glms_eval(eval, right, stack);
    }

    return (GLMSAST){.type = GLMS_AST_TYPE_STACK_PTR,
		     .as.stackptr.ptr =
			 value}; // glms_eval(eval, *value, stack);
  }

  return (GLMSAST){.type = GLMS_AST_TYPE_UNDEFINED};
}

GLMSAST glms_eval_access(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  GLMSAST right = *ast.as.access.right;

  if (right.type != GLMS_AST_TYPE_ARRAY) {
    return glms_eval_access_by_key(eval, ast, stack);
  }

  right = glms_eval(eval, *ast.as.access.right, stack);

  GLMSAST *ptr = glms_ast_get_ptr(right);

  if (ptr)
    right = *ptr;

  GLMSAST left = glms_eval(eval, *ast.as.access.left, stack);

  GLMSAST *leftptr = glms_ast_get_ptr(left);

  if (leftptr)
    left = *leftptr;

  if (left.type == GLMS_AST_TYPE_UNDEFINED) {
    GLMS_WARNING_RETURN(ast, stderr, "cannot index undefined.\n");
  }
  GLMSAST *right_value = right.children != 0 && right.children->length > 0
			     ? right.children->items[0]
			     : 0;

  GLMSAST *rightptr = right_value ? glms_ast_get_ptr(*right_value) : 0;

  if (rightptr)
    right_value = rightptr;

  GLMSAST accessor =
      right_value ? glms_eval(eval, *right_value, stack) : (GLMSAST){0};

  int64_t idx = (int64_t)glms_ast_number(accessor);

  GLMSAST *v = glms_ast_access_by_index(&left, idx, eval->env);

  if (!v)
    return ast;

  GLMSAST result = glms_eval(eval, *v, stack);
  return result;
}

GLMSAST glms_eval_function(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  // ast->as.func.id = glms_eval(eval, ast->as.func.id, stack);

  if (ast.fptr)
    return ast;

  const char *fname = glms_ast_get_name(&ast);

  GLMSAST *copy = 0;
  if (fname && !glms_stack_get(stack, fname)) {
    copy = glms_ast_copy(ast, eval->env);
    glms_stack_push(stack, fname, copy);
  }

  if (copy) {
    return (GLMSAST){.type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = copy};
  }

  return ast;
}

GLMSAST glms_eval_typedef(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  GLMSAST *id = ast.as.tdef.id;
  GLMSAST factor = glms_eval(eval, *ast.as.tdef.factor, stack);

  const char *fname = glms_ast_get_name(id);

  if (!fname)
    GLMS_WARNING_RETURN(ast, stderr, "Expected a name to exist.\n");

  if (!glms_stack_get(stack, fname)) {
    glms_stack_push(stack, fname, glms_ast_copy(factor, eval->env));
  }

  return factor;
}

GLMSAST glms_eval_struct(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  if (ast.props.initialized == false)
    return ast;

  GLMSAST *new_ast = glms_ast_copy(ast, eval->env);

  HashyIterator it = {0};
  while (hashy_map_iterate(&ast.props, &it)) {
    if (!it.bucket->key)
      continue;
    if (!it.bucket->value)
      continue;

    const char *key = it.bucket->key;
    GLMSAST *value = (GLMSAST *)it.bucket->value;

    GLMSAST eval_value = glms_eval(eval, *value, stack);
    glms_ast_object_set_property(new_ast, key,
				 glms_ast_copy(eval_value, eval->env));
  }

  GLMSAST ptr_ast =
      (GLMSAST){.type = GLMS_AST_TYPE_STACK_PTR, .as.stackptr.ptr = new_ast};

  return ptr_ast;
}

GLMSAST glms_eval_import_extension(GLMSEval *eval, GLMSAST ast,
				   GLMSStack *stack, const char *path) {
  GLMSAST result = (GLMSAST){.type = GLMS_AST_TYPE_UNDEFINED};

  if (!glms_file_exists(path))
    GLMS_WARNING_RETURN(result, stderr, "No such file `%s`\n", path);

  GLMSExtensionEntryFunc func =
      glms_load_symbol_function(path, "glms_extension_entry");

  if (!func)
    GLMS_WARNING_RETURN(result, stderr, "Could not load `%s`\n", path);

  GLMSEnv *import_env = NEW(GLMSEnv);
  glms_env_init(import_env, 0, path, eval->env->config);
  func(import_env);
  // func(eval->env);

  GLMSAST *result_ast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_STACK, false);
  result_ast->as.stack.env = import_env;

  const char *id_name =
      glms_string_view_get_value(&ast.as.import.id->as.id.value);

  glms_stack_push(stack, id_name, result_ast);

  return (GLMSAST){.type = GLMS_AST_TYPE_STACK_PTR,
		   .as.stackptr.ptr = result_ast};
  //*out = (GLMSAST){ .type = GLMS_AST_TYPE_BOOL, .as.boolean = true };
}

GLMSAST glms_eval_import(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  const char *path = glms_string_view_get_value(&ast.as.import.value);
  if (!path)
    return ast;

  const char *abspath =
      glms_file_exists(path) ? path : glms_env_get_path_for(eval->env, path);

  if (!glms_file_exists(abspath)) {
    GLMS_WARNING_RETURN((GLMSAST){.type = GLMS_AST_TYPE_UNDEFINED}, stderr,
			"No such file `%s`.\n", path);
  }

  if (strstr(abspath, ".so") != 0)
    return glms_eval_import_extension(eval, ast, stack, abspath);

  char *source = glms_get_file_contents(abspath);
  GLMSEnv *import_env = NEW(GLMSEnv);
  glms_env_init(import_env, source, abspath, eval->env->config);
  glms_env_exec(import_env);

  GLMSAST *result_ast = glms_env_new_ast(eval->env, GLMS_AST_TYPE_STACK, false);
  result_ast->as.stack.env = import_env;

  const char *id_name =
      glms_string_view_get_value(&ast.as.import.id->as.id.value);

  glms_stack_push(stack, id_name, result_ast);

  return (GLMSAST){.type = GLMS_AST_TYPE_STACK_PTR,
		   .as.stackptr.ptr = result_ast};
}

GLMSAST glms_eval_string(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  glms_eval_construct_string(eval, &ast, stack);
  return ast;
}


GLMSAST glms_eval(GLMSEval *eval, GLMSAST ast, GLMSStack *stack) {
  GLMSAST *t = glms_eval_get_type(eval, stack, &ast);

  if (t) {
    ast.to_string = t->to_string;
    ast.swizzle = t->swizzle;
    memcpy(&ast.op_overloads[0], &t->op_overloads[0],
	   sizeof(GLMSASTOperatorOverload) * GLMS_AST_OPERATOR_OVERLOAD_CAP);
  }

  switch (ast.type) {
  case GLMS_AST_TYPE_STRING: {
    return glms_eval_string(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_IMPORT: {
    return glms_eval_import(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_TYPEDEF: {
    return glms_eval_typedef(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_STRUCT: {
    return glms_eval_struct(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_ACCESS: {
    return glms_eval_access(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_BLOCK: {
    return glms_eval_block(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_CALL: {
    return glms_eval_call(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_COMPOUND: {
    return glms_eval_compound(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_BINOP: {
    return glms_eval_binop(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_UNOP: {
    return glms_eval_unop(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_ID: {
    return glms_eval_id(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_FOR: {
    return glms_eval_for(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_STACK_PTR: {
    return glms_eval_stack_ptr(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_FUNC: {
    return glms_eval_function(eval, ast, stack);
  }; break;
  default: {
    return ast;
  }; break;
  }

  return (GLMSAST){.type = GLMS_AST_TYPE_NOOP};
}

bool glms_eval_expect(GLMSEval *eval, GLMSStack *stack, GLMSASTType *types,
		      int nr_types, GLMSASTBuffer *args) {
  if (!eval || !stack)
    return false;
  if (!types || nr_types <= 0)
    return true;

  if (args == 0 || (args->length != nr_types))
    GLMS_WARNING_RETURN(false, stderr,
			"Expected `%d` arguments but got `%ld`.\n", nr_types,
			args ? args->length : 0);

  for (int i = 0; i < nr_types; i++) {
    GLMSAST arg = args->items[i];
    GLMSAST *ptr = glms_ast_get_ptr(arg);
    if (ptr != 0)
      arg = *ptr;

    if (arg.type != types[i]) {
      GLMS_WARNING_RETURN(
	  false, stderr, "Expected `%s` at arg `%d` but got `%s`.\n",
	  GLMS_AST_TYPE_STR[types[i]], i, GLMS_AST_TYPE_STR[arg.type]);
    }
  }

  return true;
}
