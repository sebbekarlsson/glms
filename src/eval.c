#include "glms/ast.h"
#include "glms/ast_type.h"
#include "glms/stack.h"
#include "glms/token.h"
#include "hashy/keylist.h"
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
  eval->arena = false;
  return 1;
}

GLMSAST *glms_eval_lookup(GLMSEval *eval, const char *name, GLMSStack *stack) {
  GLMSAST *stack_ast = glms_stack_get(stack, name);

  if (stack_ast)
    return stack_ast;

  GLMSAST *t = glms_env_lookup_type(eval->env, name);

  if (t)
    return t;

  GLMSAST *ast = glms_env_lookup_function(eval->env, name);

  if (ast)
    return ast;

  return 0;
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
  case GLMS_AST_TYPE_STACK_PTR: {
    return glms_eval_stack_ptr(eval, ast, stack);
  }; break;
  case GLMS_AST_TYPE_ARRAY: {
    return glms_eval_array(eval, ast, stack);
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

static GLMSAST *glms_eval_id_private(GLMSEval *eval, GLMSAST *ast,
                                     GLMSStack *stack) {

  const char *name = glms_ast_get_name(ast);
  GLMSAST *value = glms_eval_lookup(eval, name, stack);
  GLMSAST *tast = glms_ast_get_type(ast);

  if (value != 0)
    return value;

  if (tast != 0 && value == 0 && name != 0 && ast->flags != 0 &&
      ast->flags->length > 0) {

    const char *tast_name = glms_ast_get_name(tast);
    GLMSAST *tvalue = glms_eval_lookup(eval, tast_name, stack);

    if (tvalue == 0) {
      switch (tast->as.id.op) {
      case GLMS_TOKEN_TYPE_SPECIAL_NUMBER: {
        return glms_env_new_ast_number(eval->env, 0.0f, eval->arena);
      }; break;
      case GLMS_TOKEN_TYPE_SPECIAL_STRING: {
        return glms_env_new_ast_string(eval->env, 0, eval->arena);
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

GLMSAST *glms_eval_id(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  GLMSAST *id = glms_eval_id_private(eval, ast, stack);
  glms_env_apply_type(eval->env, eval, stack, id);

  return id;
}

GLMSAST *glms_eval_call(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  const char *fname =
      glms_string_view_get_value(&ast->as.call.left->as.id.value);

  GLMSAST *func = fname ? glms_eval_lookup(eval, fname, stack) : 0;

  if (!func && ast->as.call.func != 0) {
    func = ast->as.call.func;
  }

  if (func->constructor) {
    return glms_eval(eval, func->constructor(eval, stack, ast->children, 0),
                     stack);
  }

  // constructor
  if (func->type == GLMS_AST_TYPE_STRUCT) {
    GLMSAST *copied = glms_ast_copy(*func, eval->env);

    GLMSStack tmp_stack = {0};
    glms_stack_init(&tmp_stack);
    glms_stack_copy(*stack, &tmp_stack);

    if (ast->children != 0 && func->props.initialized) {
      HashyIterator it = {0};

      int64_t i = 0;
      while (hashy_map_iterate(&func->props, &it)) {
        if (!it.bucket->key)
          continue;
        if (!it.bucket->value)
          continue;

        const char *key = it.bucket->key;
        GLMSAST *value =
            glms_eval(eval, (GLMSAST *)it.bucket->value, &tmp_stack);

        GLMSAST *arg_value = glms_eval(
            eval, ast->children->items[MAX(0, (ast->children->length - 1) - i)],
            &tmp_stack);
        glms_ast_object_set_property(copied, key, arg_value);
        i++;

        if (i >= ast->children->length) {
          hashy_key_list_clear(&it.keys);
          break;
        }
      }
    }

    GLMSAST *result = glms_eval(eval, copied, stack);
    glms_ast_keep(result);
    glms_stack_clear(&tmp_stack);
    return result;
  }

  if (func->type != GLMS_AST_TYPE_FUNC) {
    GLMS_WARNING_RETURN(ast, stderr, "`%s` is not callable.\n",
                        GLMS_AST_TYPE_STR[func->type]);
    func = 0;
  }

  if (!func)
    GLMS_WARNING_RETURN(ast, stderr, "No such function `%s`\n", fname);

  if (func->fptr != 0) {
    glms_eval_enable_arena(eval);
    GLMSAST *self = glms_stack_get(stack, "self");
    GLMSAST *result = func->fptr(eval, self ? self : ast, ast->children, stack);
    glms_env_apply_type(eval->env, eval, stack, result);
    glms_ast_keep(result);
    return result;
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




  if (left->swizzle != 0) {
    GLMSAST *result = 0;
    if ((result = left->swizzle(eval, stack, left, right))) {
      return glms_eval(eval, result, stack);
    }
  }

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
      glms_stack_push(stack, "self", left);
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
    ast->as.unop.left = glms_eval(eval, ast->as.unop.left, stack);
    return glms_env_new_ast_number(eval->env, -GLMSAST_VALUE(ast->as.unop.left),
                                   eval->arena);
  }; break;
  case GLMS_TOKEN_TYPE_ADD: {
    ast->as.unop.left = glms_eval(eval, ast->as.unop.left, stack);
    return glms_env_new_ast_number(eval->env, +GLMSAST_VALUE(ast->as.unop.left),
                                   eval->arena);
  }; break;
  case GLMS_TOKEN_TYPE_ADD_ADD: {
    GLMSAST *left = glms_eval(eval, ast->as.unop.left, stack);
    left->as.number.value++;
    return left;
  }; break;
  case GLMS_TOKEN_TYPE_SUB_SUB: {
    ast->as.unop.left = glms_eval(eval, ast->as.unop.left, stack);
    ast->as.unop.left->as.number.value--;
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
    ast->as.unop.right = glms_eval(eval, ast->as.unop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   -GLMSAST_VALUE(ast->as.unop.right), eval->arena);
  }; break;
  case GLMS_TOKEN_TYPE_ADD: {
    ast->as.unop.right = glms_eval(eval, ast->as.unop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   +GLMSAST_VALUE(ast->as.unop.right), eval->arena);
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

GLMSAST *glms_eval_binop_mul_number_number(GLMSEval *eval, GLMSAST *left,
                                           GLMSAST *right, GLMSStack *stack) {

  float next_value = GLMSAST_VALUE(left) * GLMSAST_VALUE(right);

  if (next_value == GLMSAST_VALUE(left)) return left; 
  
  return glms_env_new_ast_number(
      eval->env, next_value, eval->arena);
}

GLMSAST *glms_eval_binop_mul_array_array(GLMSEval *eval, GLMSAST *ast,
                                         GLMSStack *stack) {
  GLMSAST *left = ast->as.binop.left;
  GLMSAST *right = ast->as.binop.right;

  int64_t len_left = glms_ast_array_get_length(left);
  int64_t len_right = glms_ast_array_get_length(right);
  int64_t len_min = MIN(len_left, len_right);

  if (len_min <= 0)
    return ast;

  GLMSAST *new_array = glms_env_new_ast(eval->env, GLMS_AST_TYPE_ARRAY, eval->arena);

  for (int64_t i = 0; i < len_min; i++) {
    GLMSAST *chleft = left->children->items[i];
    GLMSAST *chright = right->children->items[i];

    glms_ast_push(new_array, glms_eval_binop_mul_number_number(eval, chleft,
                                                               chright, stack));
  }

  return new_array;
}

GLMSAST *glms_eval_binop_mul(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  GLMSAST *left= glms_eval(eval, ast->as.binop.left, stack);
  GLMSAST *right =  glms_eval(eval, ast->as.binop.right, stack);

  if (left->type == GLMS_AST_TYPE_NUMBER &&
      right->type == GLMS_AST_TYPE_NUMBER) {
    return glms_eval_binop_mul_number_number(eval, left, right, stack);
  } else if (left->type == GLMS_AST_TYPE_ARRAY &&
             right->type == GLMS_AST_TYPE_ARRAY) {
    return glms_eval_binop_mul_array_array(eval, ast, stack);
  } else if (left->type == GLMS_AST_TYPE_BOOL && right->type == GLMS_AST_TYPE_NUMBER) {
    return glms_env_new_ast_number(eval->env, ((float)left->as.boolean) * GLMSAST_VALUE(right), eval->arena);
  } else if (left->type == GLMS_AST_TYPE_NUMBER && right->type == GLMS_AST_TYPE_BOOL) {
    return glms_env_new_ast_number(eval->env, GLMSAST_VALUE(left) * (float)right->as.boolean, eval->arena);
  }

  return ast;
}

GLMSAST *glms_eval_binop(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {

  glms_eval_enable_arena(eval);
  
  GLMSAST* left = glms_eval(eval, ast->as.binop.left, stack);
  GLMSAST* right = glms_eval(eval, ast->as.binop.right, stack);


  GLMSTokenType op = ast->as.binop.op;

  GLMSASTOperatorOverload op_fptr = left->op_overloads[op % GLMS_AST_OPERATOR_OVERLOAD_CAP];

  if (op_fptr) {
    GLMSAST* result = op_fptr(eval, stack, left, right);
    result = result ? glms_eval(eval, result, stack) : 0;
    result = result ? glms_env_apply_type(eval->env, eval, stack, result) : 0;

    if (result) return result;
  }

  switch (ast->as.binop.op) {
  case GLMS_TOKEN_TYPE_MUL: {
    return glms_eval_binop_mul(eval, ast, stack);
  }; break;
  case GLMS_TOKEN_TYPE_DIV: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   GLMSAST_VALUE(ast->as.binop.left) /
                                       GLMSAST_VALUE(ast->as.binop.right),
                                   eval->arena);
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
    GLMSAST* left = glms_eval(eval, ast->as.binop.left, stack);
    GLMSAST* right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   GLMSAST_VALUE(left) +
                                       GLMSAST_VALUE(right),
                                   eval->arena);
  }; break;
  case GLMS_TOKEN_TYPE_SUB: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   GLMSAST_VALUE(ast->as.binop.left) -
                                       GLMSAST_VALUE(ast->as.binop.right),
                                   eval->arena);
  }; break;
  case GLMS_TOKEN_TYPE_PERCENT: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   (int)GLMSAST_VALUE(ast->as.binop.left) %
                                       (int)GLMSAST_VALUE(ast->as.binop.right),
                                   eval->arena);
  }; break;
  case GLMS_TOKEN_TYPE_EQUALS_EQUALS: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(eval->env,
                                   (float)glms_ast_compare_equals_equals(
                                       ast->as.binop.left, ast->as.binop.right),
                                   eval->arena);
  }; break;
  case GLMS_TOKEN_TYPE_GT: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(
        eval->env,
        (float)glms_ast_compare_gt(ast->as.binop.left, ast->as.binop.right),
        eval->arena);
  }; break;
  case GLMS_TOKEN_TYPE_GTE: {
    GLMSAST *left = glms_eval(eval, ast->as.binop.left, stack);
    GLMSAST *right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(
        eval->env, (float)glms_ast_compare_gte(left, right), eval->arena);
  }; break;
  case GLMS_TOKEN_TYPE_LT: {
    GLMSAST *left = glms_eval(eval, ast->as.binop.left, stack);
    GLMSAST *right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(
        eval->env, (float)glms_ast_compare_lt(left, right), eval->arena);
  }; break;
  case GLMS_TOKEN_TYPE_LTE: {
    ast->as.binop.left = glms_eval(eval, ast->as.binop.left, stack);
    ast->as.binop.right = glms_eval(eval, ast->as.binop.right, stack);
    return glms_env_new_ast_number(
        eval->env,
        (float)glms_ast_compare_lte(ast->as.binop.left, ast->as.binop.right),
        eval->arena);
  }; break;
  case GLMS_TOKEN_TYPE_EQUALS: {

    if (ast->as.binop.right->type == GLMS_AST_TYPE_FUNC &&
        !glms_ast_get_name(ast->as.binop.right)) {
      if (ast->as.binop.left->type == GLMS_AST_TYPE_ID) {
        ast->as.binop.right->as.func.id = ast->as.binop.left;
      }
    }

    GLMSAST *typeflag = glms_ast_get_type(ast->as.binop.left);

    const char *name = glms_ast_get_name(ast);
    GLMSAST *left = ast->as.binop.left;
    GLMSAST *right = ast->as.binop.right;

    if (!typeflag) {
      left = glms_eval(eval, left, stack);
    }
    right = glms_eval(eval, right, stack);

    if (typeflag || (ast->as.binop.left->type == GLMS_AST_TYPE_ID && !glms_eval_lookup(eval, name, stack))) {
      GLMSAST *copy = glms_ast_copy(*right, eval->env);
      glms_stack_push(stack, name, copy);
      left = copy;
    }

    glms_stack_push(stack, name, left);
    //   GLMSAST* existing = 0;

    // if (name) {
    // existing = glms_eval_lookup(eval, name, stack);
    // }

    GLMSAST *type = 0;

    if (typeflag != 0) {
      type = glms_env_lookup_type(eval->env, glms_ast_get_name(typeflag));
    }

    //  if (name != 0 && existing == 0) {

    //  if (typeflag) {
    //    glms_stack_push(stack, name, right);
    // }
    // }

    // left = glms_eval(eval, left, stack);

    bool same_type = (left->type == right->type);

    if (same_type) {
      switch (left->type) {
      case GLMS_AST_TYPE_NUMBER: {
        left->as.number.value = right->as.number.value;
      }; break;
      case GLMS_AST_TYPE_STRING: {
        const char *strval = glms_ast_get_string_value(right);
        if (strval != 0) {
          if (left->as.string.heap != 0) {
            free(left->as.string.heap);
            left->as.string.heap = 0;
          }

          left->as.string.heap = strdup(strval);
        }
        // left->as.string.value = right->as.string.value;
      }; break;
      default: {
        // TODO: This is most definitely a memory leak.
        //*left = *right;
      }; break;
      }
    }

    left->value_type = left->value_type ? left->value_type : type;

    if (stack->return_flag) {
      GLMSAST *retval = glms_eval_lookup(eval, "return", stack);
      if (retval)
        return retval;
    }

    //      glms_stack_push(stack, name, left);

    return left;
  }; break;
  default: {
    return ast;
  }; break;
  }

  return ast;
}

GLMSAST *glms_eval_function(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  // ast->as.func.id = glms_eval(eval, ast->as.func.id, stack);

  if (ast->fptr)
    return ast;

  const char *fname = glms_ast_get_name(ast);


  if (fname) {
    glms_stack_push(stack, fname, ast);
  }

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

GLMSAST *glms_eval_block_switch(GLMSEval *eval, GLMSAST *ast,
                                GLMSStack *stack) {
  if (!ast->as.block.body || !ast->as.block.expr)
    return ast;

  GLMSAST *body = ast->as.block.body;

  if (!body->children || body->children->length <= 0)
    return ast;

  GLMSAST *expr = glms_eval(eval, ast->as.block.expr, stack);

  for (int64_t i = 0; i < body->children->length; i++) {
    GLMSAST *child = body->children->items[i];
    if (child->type != GLMS_AST_TYPE_BLOCK)
      GLMS_WARNING_RETURN(ast, stderr, "Invalid switch body item.\n");
    if (!child->as.block.expr || !child->as.block.body)
      continue;

    GLMSAST *child_expr = glms_eval(eval, child->as.block.expr, stack);

    if (glms_ast_compare_equals_equals(expr, child_expr)) {
      return glms_eval(eval, child->as.block.body, stack);
    }
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
  case GLMS_TOKEN_TYPE_SPECIAL_SWITCH: {
    return glms_eval_block_switch(eval, ast, stack);
  }; break;
  default: {
    return ast;
  }; break;
  }

  return ast;
}

GLMSAST *glms_eval_for(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  if (!ast->as.forloop.body) {
    return ast;
  }
  if (ast->children == 0 || ast->children->length <= 0) {
    return ast;
  }

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

GLMSAST *glms_eval_array(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  if (!ast->children || ast->children->length <= 0)
    return ast;

  for (int64_t i = 0; i < ast->children->length; i++) {
    ast->children->items[i] = glms_eval(eval, ast->children->items[i], stack);
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
    glms_eval_disable_arena(eval);

    child = glms_eval(eval, child, &local_stack);

    if (local_stack.return_flag) {
      GLMSAST *retval = glms_eval_lookup(eval, "return", &local_stack);
      if (retval != 0) {
        local_stack.return_flag = false;
        glms_stack_clear(&local_stack);
	glms_eval_disable_arena(eval);
        return retval;
      }
    }
  }

  if (stack->depth <= 0) {
    glms_stack_copy(local_stack, stack);
  }

  glms_ast_keep(ast);
  glms_stack_clear(&local_stack);
  glms_stack_clear_trash(stack);
  glms_eval_disable_arena(eval);

  return ast;
}

int glms_eval_ast_list(GLMSEval *eval, GLMSASTList *list, GLMSStack *stack) {
  if (!list) return 0;
  if (list->length <= 0 || list->items == 0) return 0;


  for (int64_t i = 0; i < list->length; i++) {
    list->items[i] = glms_eval(eval, list->items[i], stack);
  }

  return 1;
}

GLMSAST *glms_eval_stack_ptr(GLMSEval *eval, GLMSAST *ast, GLMSStack *stack) {
  int idx = ast->as.stackptr.idx;

  if (idx < 0 || idx >= stack->tiny_stack_length) return ast;


  GLMSAST ptr = stack->tiny_stack[idx];

  *ast = ptr;

  return ast;
}

void glms_eval_enable_arena(GLMSEval *eval) {
  if (!eval) return;
  eval->arena = true;
}
void glms_eval_disable_arena(GLMSEval *eval) {
  if (!eval) return;
  eval->arena = false;
}
