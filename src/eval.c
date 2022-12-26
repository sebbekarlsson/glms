#include <glms/builtin.h>
#include <glms/env.h>
#include <glms/eval.h>
#include <glms/macros.h>

int glms_eval_init(GLMSEval *eval, struct GLMS_ENV_STRUCT *env) {
  if (!eval || !env)
    return 0;
  if (eval->initialized)
    return 1;
  eval->initialized = true;
  eval->env = env;
  return 1;
}

GLMSAST *glms_eval_lookup(GLMSEval *eval, const char *name, GLMSStack *stack) {

  GLMSAST *ast = glms_env_lookup_function(eval->env, name);

  if (ast)
    return ast;

  return glms_stack_get(stack, name);
}

GLMSAST *glms_eval(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  if (!eval || !ast)
    return 0;
  if (!eval->initialized)
    GLMS_WARNING_RETURN(0, stderr, "evaluator not initialized.\n");

  switch (ast->type) {
  case GLMS_AST_TYPE_COMPOUND: {
    return glms_eval_compound(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_BINOP: {
    return glms_eval_binop(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_UNOP: {
    return glms_eval_unop(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_CALL: {
    return glms_eval_call(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_FUNC: {
    return glms_eval_function(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_TYPEDEF: {
    return glms_eval_typedef(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_STRUCT: {
    return glms_eval_struct(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_BLOCK: {
    return glms_eval_block(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_FOR: {
    return glms_eval_for(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_ID: {
    return glms_eval_id(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_ACCESS: {
    return glms_eval_access(eval, ast, stack);
  }; break;
  default: {
    return ast;
  }; break;
  }

  return ast;
}

GLMSAST *glms_eval_struct(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  if (ast->props.initialized == false)
    return ast;

  HashyIterator it = {0};

  while (hashy_map_iterate(&ast->props, &it)) {
    if (!it.bucket->key)
      continue;
    if (!it.bucket->value)
      continue;

    const char *key = it.bucket->key;
    GLMSAST *value = (GLMSAST *)it.bucket->value;

    value = glms_eval(eval, value, stack);
    glms_ast_object_set_property(ast, key, value);
  }

  return ast;
}

GLMSAST *glms_eval_id(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {

  const char *name = glms_ast_get_name(ast);
  GLMSAST *value = glms_eval_lookup(eval, name, stack);
  GLMSAST *tast = glms_ast_get_type(ast);

  if (value != 0)
    return value;

  if (tast != 0 && value == 0 && name != 0 & ast->flags != 0 &&
      ast->flags->length > 0) {

    const char *tast_name = glms_ast_get_name(tast);
    GLMSAST *tvalue = glms_eval_lookup(eval, tast_name, stack);

    if (tvalue == 0) {
      switch (tast->as.id.op) {
      case GLMS_TOKEN_TYPE_SPECIAL_NUMBER: {
        return glms_env_new_ast_number(eval->env, 0.0f);
      }; break;
      case GLMS_TOKEN_TYPE_SPECIAL_STRING: {
        return glms_env_new_ast_string(eval->env, "");
      }; break;
      default: {
        return ast;
      }; break;
      }
      return ast;
    } else if (tvalue != 0) {
      GLMSAST *copied = glms_ast_copy(*tvalue, eval->env);
      glms_stack_push(stack, name, copied);

      return glms_eval(eval, copied, stack);
    }
  }

  if (!name)
    return ast;

  if (!value) {
    GLMS_WARNING_RETURN(eval->env->undefined, stderr, "`%s` is undefined.\n",
                        name);
  }

  return value;
}

GLMSAST *glms_eval_call(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  const char *fname =
      glms_string_view_get_value(&ast->as.call.left->as.id.value);

  if (!fname) {
    GLMS_WARNING_RETURN(ast, stderr, "function missing name.\n");
  }

  GLMSAST *func = glms_eval_lookup(eval, fname, stack);

  if (!func && ast->as.call.func != 0) {
    func = ast->as.call.func;
  }

  if (func->type != GLMS_AST_TYPE_FUNC)
    func = 0;

  if (!func)
    GLMS_WARNING_RETURN(ast, stderr, "No such function `%s`\n", fname);

  if (func->fptr != 0) {
    return func->fptr(eval, ast, ast->children, stack);
  } else if (func->as.func.body != 0) {

    GLMSStack tmp_stack = {0};
    glms_stack_init(&tmp_stack);
    glms_stack_copy(*stack, &tmp_stack);

    if (ast->children != 0 && func->children != 0) {
      for (int64_t i = 0;
           i < MIN(ast->children->length, func->children->length); i++) {
        GLMSAST *arg_value =
            glms_eval(eval, ast->children->items[i], &tmp_stack);
        GLMSAST *arg_func = func->children->items[i];

        const char *arg_name = glms_ast_get_name(arg_func);
        if (!arg_name)
          continue;

        glms_stack_push(&tmp_stack, arg_name, arg_value);
      }
    }

    GLMSAST *result = glms_eval(eval, func->as.func.body, &tmp_stack);
    glms_stack_clear(&tmp_stack);
    return result;
  } else {
    return ast;
  }

  return ast;
}

GLMSAST *glms_eval_access(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  if (ast->as.access.left->type == GLMS_AST_TYPE_UNDEFINED)
    GLMS_WARNING_RETURN(ast, stderr, "cannot index undefined.\n");
  GLMSAST *left = glms_eval(eval, ast->as.access.left, stack);
  GLMSAST *right = ast->as.access.right;

  if (right->type != GLMS_AST_TYPE_ARRAY) {
    const char *key = glms_ast_get_string_value(right);
    if (!key) {
      return glms_eval(eval, right, stack);
    }
    GLMSAST *value = glms_ast_access_by_key(left, key, eval->env);

    if (!value)
      return glms_eval(eval, right, stack);

    if (value->type == GLMS_AST_TYPE_FUNC &&
        right->type == GLMS_AST_TYPE_CALL) {
      right->as.call.func = value;
      return glms_eval(eval, right, stack);
    }
    return glms_eval(eval, value, stack);
  }

  GLMSAST *right_value = right->children != 0 && right->children->length > 0
                             ? right->children->items[0]
                             : 0;

  right_value = right_value ? glms_eval(eval, right_value, stack) : 0;

  int64_t idx = right_value ? (int64_t)(GLMSAST_VALUE(right_value)) : 0;

  return glms_eval(eval, glms_ast_access_by_index(left, idx, eval->env), stack);
}

GLMSAST *glms_eval_unop_left(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  switch (ast->as.unop.op) {
  case GLMS_TOKEN_TYPE_SUB: {
    ast->as.binop.left = glms_eval(eval, ast->as.unop.left, stack);
    return glms_env_new_ast_number(eval->env,
                                   -GLMSAST_VALUE(ast->as.unop.left));
  }; break;
  case GLMS_TOKEN_TYPE_ADD: {
    ast->as.binop.left = glms_eval(eval, ast->as.unop.left, stack);
    return glms_env_new_ast_number(eval->env,
                                   +GLMSAST_VALUE(ast->as.unop.left));
  }; break;
  case GLMS_TOKEN_TYPE_ADD_ADD: {
    ast->as.binop.left = glms_eval(eval, ast->as.unop.left, stack);
    ast->as.binop.left->as.number.value++;
  }; break;
  case GLMS_TOKEN_TYPE_SUB_SUB: {
    ast->as.binop.left = glms_eval(eval, ast->as.unop.left, stack);
    ast->as.binop.left->as.number.value--;
  }; break;
  default: {
    return ast;
  }; break;
  }
  return ast;
}

GLMSAST *glms_eval_unop_right(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  switch (ast->as.unop.op) {
  case GLMS_TOKEN_TYPE_SPECIAL_RETURN: {
    GLMSAST *retval = glms_eval(eval, ast->as.unop.right, stack);
    glms_stack_push(stack, "return", retval);
    stack->return_flag = true;
    return retval;
  }; break;
  case GLMS_TOKEN_TYPE_SUB: {
    ast->as.binop.right = glms_eval(eval, ast->as.unop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   -GLMSAST_VALUE(ast->as.unop.right));
  }; break;
  case GLMS_TOKEN_TYPE_ADD: {
    ast->as.binop.right = glms_eval(eval, ast->as.unop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   +GLMSAST_VALUE(ast->as.unop.right));
  }; break;
  default: {
    return ast;
  }; break;
  }
  return ast;
}

GLMSAST *glms_eval_unop(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  if (ast->as.unop.left)
    return glms_eval_unop_left(eval, ast, stack);
  return glms_eval_unop_right(eval, ast, stack);
}

GLMSAST *glms_eval_binop(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  switch (ast->as.binop.op) {
  case GLMS_TOKEN_TYPE_MUL: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   GLMSAST_VALUE(ast->as.binop.left) *
                                       GLMSAST_VALUE(ast->as.binop.right));
  }; break;
  case GLMS_TOKEN_TYPE_DIV: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   GLMSAST_VALUE(ast->as.binop.left) /
                                       GLMSAST_VALUE(ast->as.binop.right));
  }; break;
  case GLMS_TOKEN_TYPE_ADD_EQUALS: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    ast->as.binop.left->as.number.value += ast->as.binop.right->as.number.value;
    return ast->as.binop.left;
  }; break;
  case GLMS_TOKEN_TYPE_SUB_EQUALS: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    ast->as.binop.left->as.number.value -= ast->as.binop.right->as.number.value;
    return ast->as.binop.left;
  }; break;
  case GLMS_TOKEN_TYPE_ADD: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   GLMSAST_VALUE(ast->as.binop.left) +
                                       GLMSAST_VALUE(ast->as.binop.right));
  }; break;
  case GLMS_TOKEN_TYPE_SUB: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   GLMSAST_VALUE(ast->as.binop.left) -
                                       GLMSAST_VALUE(ast->as.binop.right));
  }; break;
  case GLMS_TOKEN_TYPE_PERCENT: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   (int)GLMSAST_VALUE(ast->as.binop.left) %
                                       (int)GLMSAST_VALUE(ast->as.binop.right));
  }; break;
  case GLMS_TOKEN_TYPE_EQUALS_EQUALS: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(
        eval->env, (float)glms_ast_compare_equals_equals(ast->as.binop.left,
                                                         ast->as.binop.right));
  }; break;
  case GLMS_TOKEN_TYPE_GT: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(
        eval->env,
        (float)glms_ast_compare_gt(ast->as.binop.left, ast->as.binop.right));
  }; break;
  case GLMS_TOKEN_TYPE_GTE: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(
        eval->env,
        (float)glms_ast_compare_gte(ast->as.binop.left, ast->as.binop.right));
  }; break;
  case GLMS_TOKEN_TYPE_LT: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(
        eval->env,
        (float)glms_ast_compare_lt(ast->as.binop.left, ast->as.binop.right));
  }; break;
  case GLMS_TOKEN_TYPE_LTE: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(
        eval->env,
        (float)glms_ast_compare_lte(ast->as.binop.left, ast->as.binop.right));
  }; break;
  case GLMS_TOKEN_TYPE_EQUALS: {

    if (ast->as.binop.right->type == GLMS_AST_TYPE_FUNC &&
        !glms_ast_get_name(ast->as.binop.right)) {
      if (ast->as.binop.left->type == GLMS_AST_TYPE_ID) {
        ast->as.binop.right->as.func.id = ast->as.binop.left;
      }
    }

    const char *name = glms_ast_get_name(ast);
    GLMSAST *left = ast->as.binop.left;
    GLMSAST *right = ast->as.binop.right =
        glms_eval(eval, ast->as.binop.right, stack);

    if (name != 0) {
      glms_stack_push(stack, name, ast->as.binop.right);
    }

    left = glms_eval(eval, left, stack);
    bool same_type = (left->type == right->type);

    if (same_type) {
      switch (ast->as.binop.left->type) {
      case GLMS_AST_TYPE_NUMBER: {
        left->as.number.value = right->as.number.value;
      }; break;
      case GLMS_AST_TYPE_STRING: {
        left->as.string.value = right->as.string.value;
      }; break;
      default: {
        // TODO: This is most definitely a memory leak.
        *left = *right;
      }; break;
      }
    }

    if (stack->return_flag) {
      GLMSAST *retval = glms_eval_lookup(eval, "return", stack);
      if (retval)
        return retval;
    }

    return right;
  }; break;
  default: {
    return ast;
  }; break;
  }

  return ast;
}

GLMSAST *glms_eval_function(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  // ast->as.func.id = glms_eval(eval, ast->as.func.id, stack);

  const char *fname = glms_ast_get_name(ast);

  if (!fname)
    GLMS_WARNING_RETURN(ast, stderr, "Expected a name to exist.\n");

  glms_stack_push(stack, fname, ast);

  return ast;
}

GLMSAST *glms_eval_typedef(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  GLMSAST *id = ast->as.tdef.id;
  GLMSAST *factor = glms_eval(eval, ast->as.tdef.factor, stack);

  const char *fname = glms_ast_get_name(id);

  if (!fname)
    GLMS_WARNING_RETURN(ast, stderr, "Expected a name to exist.\n");

  glms_stack_push(stack, fname, factor);

  return factor;
}

GLMSAST *glms_eval_block_condition(GLMSEval *eval, GLMSAST *ast,
                                   GLMSStack *stack) {
  if (ast->as.block.expr) {
    GLMSAST *expr = glms_eval(eval, ast->as.block.expr, stack);

    if (ast->as.block.body && glms_ast_is_truthy(expr)) {
      return glms_eval(eval, ast->as.block.body, stack);
    } else {
      if (ast->as.block.next) {
        return glms_eval(eval, ast->as.block.next, stack);
      }
      return ast;
    }
  }

  if (ast->as.block.next) {
    return glms_eval(eval, ast->as.block.next, stack);
  }

  return glms_eval(eval, ast->as.block.body, stack);
}

GLMSAST *glms_eval_block_while(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  if (!ast->as.block.body || !ast->as.block.expr)
    return ast;

  while (glms_ast_is_truthy(glms_eval(eval, ast->as.block.expr, stack))) {
    glms_eval(eval, ast->as.block.body, stack);
  }

  return ast;
}

GLMSAST *glms_eval_block(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  switch (ast->as.block.op) {
  case GLMS_TOKEN_TYPE_SPECIAL_WHILE: {
    return glms_eval_block_while(eval, ast, stack);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_IF: {
    return glms_eval_block_condition(eval, ast, stack);
  }; break;
  case GLMS_TOKEN_TYPE_SPECIAL_ELSE: {
    return glms_eval_block_condition(eval, ast, stack);
  }; break;
  default: {
    return ast;
  }; break;
  }

  return ast;
}

GLMSAST *glms_eval_for(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  if (!ast->as.forloop.body)
    return ast;
  if (ast->children == 0 || ast->children->length <= 0)
    return ast;

  JAST *init = ast->children->items[0];
  JAST *cond = ast->children->items[1];
  JAST *step = ast->children->items[2];

  for (glms_eval(eval, init, stack);
       glms_ast_is_truthy(glms_eval(eval, cond, stack));
       glms_eval(eval, step, stack)) {
    glms_eval(eval, ast->as.forloop.body, stack);
  }

  return ast;
}

GLMSAST *glms_eval_compound(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  if (!ast->children || ast->children->length <= 0)
    return ast;

  GLMSStack local_stack = {0};
  local_stack.depth = stack->depth + 1;
  glms_stack_init(&local_stack);
  glms_stack_copy(*stack, &local_stack);

  for (int64_t i = 0; i < ast->children->length; i++) {
    GLMSAST *child = ast->children->items[i];

    child = glms_eval(eval, child, &local_stack);

    if (local_stack.return_flag) {
      GLMSAST *retval = glms_eval_lookup(eval, "return", &local_stack);
      if (retval != 0) {
        local_stack.return_flag = false;
        glms_stack_clear(&local_stack);
        return retval;
      }
    }

    if (ast->children->length == 1) {
      glms_stack_clear(&local_stack);
      return child;
    }
  }

  if (stack->depth <= 0) {
    glms_stack_copy(local_stack, stack);
  }

  glms_stack_clear(&local_stack);

  return ast;
}
