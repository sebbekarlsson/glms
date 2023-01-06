# Integration
> The reason this language was created was to use it in a game engine.  
> So the following examples will cover this.

## Example
> Let's say you have a struct in your project that looks like this:
```C
// could also be a class, or anything really.
typedef struct {
  Vector3 position;
} GameObject;
```
> And you want to use GLMS to manipulate an instance of this object somehow.  
> Here's how you would do this:
```C
#include <glms/env.h>
#include <glms/ast.h>

typedef struct {
  Vector3 position;
} GameObject;


// function to get position
int game_object_fptr_get_position(GLMSEval *eval, GLMSAST *self,
                                  GLMSASTBuffer* args, GLMSStack* stack,
                                  GLMSAST *out) {

  GameObject* obj = (GameObject*)self->ptr; // `ptr` is basically free real-estate, you can assign whatever you want to it.
                                            // here we already have assigned an instance of `GameObject` to it.

  *out = (GLMSAST){ .type = GLMS_AST_TYPE_VEC3, .as.v3 = obj->position }; // we return a vec3

  return 1; // 1 indicates success
}

// function to modify & set position
int game_object_fptr_set_position(GLMSEval *eval, GLMSAST *self,
                                  GLMSASTBuffer* args, GLMSStack* stack,
                                  GLMSAST *out) {

  if (!glms_eval_expect(eval, stack, (GLMSASTType[]){ GLMS_AST_TYPE_VEC3 }, 1, args)) return 0; // we expect to receive a vec3
  
  GameObject* obj = (GameObject*)self->ptr; // `ptr` is basically free real-estate, you can assign whatever you want to it.
                                            // here we already have assigned an instance of `GameObject` to it.

  obj->position = args->items[0].as.v3; // reassign value

  return 1; // 1 indicates success
}

// constructor for our type
void game_object_fptr_constructor(GLMSEval *eval, GLMSStack *stack,
                                  GLMSASTBuffer *args, GLMSAST *self) {
  self->constructor = game_object_fptr_constructor; // make sure GLMS knows how to re-construct instances of this type.
  glms_ast_register_function(eval->env, self, "getPosition", game_object_fptr_get_position);
  glms_ast_register_function(eval->env, self, "setPosition", game_object_fptr_set_position);
  
}

// register our custom type
void register_custom_types(GLMSEnv* env) {
  GLMSAST* my_type = glms_env_new_ast(env, GLMS_AST_TYPE_STRUCT, true);
  glms_env_register_type(env, "GameObject", my_type, game_object_fptr_constructor,
			 0, // swizzle,
			 0, // to_string,
			 0  // destructor
			 );
  
}

// expose an instance of our `GameObject` to the scripting environment
void expose_game_object(GLMSEnv* env, GameObject* obj) {
  GLMSAST* instance = glms_env_new_ast(env, GLMS_AST_TYPE_STRUCT, true);
  instance->constructor = game_object_fptr_constructor; // Again, this is our way of letting GLMS know what type this is. 
  // alternatively, you can also do this:
  // instance->typename = strdup("GameObject");
  // and GLMS will automatically find the constructor for you.
  // but we can save some bytes here by not allocating a new string.
  

  instance->ptr = obj; // `ptr` is free real-estate, assign whatever you want to it.
  glms_env_register_any(env, "myobj", instance); // will be available as a global variable called "myobj"
}

// finally setup everything
void some_place_in_your_project_where_setting_up_the_scripting_environment_is_appropriate() {
  GLMSEnv env = {0};
  const char* source_code = get_source_code_of_script_from_somewhere();
  glms_env_init(
		&env,
		source_code, // source code of script
		"path/to/where/source/code/originates", // this is optional, put a `0` if you don't want to use this.
		                                        // It's used to have the scripting environment more intelligently
		                                        // find "import" paths.
		(GLMSConfig){0} // configuration for environment,
		                // currently this can only be empty.
		);



  GameObject* obj = get_this_object_from_somewhere_maybe_malloc();
  
  register_custom_types(&env);
  expose_game_object(&env, obj);
  
  // later execute the script by running
  glms_env_exec(&env);
  
  
  // when you're done, run:
  glms_env_clear(&env);
  // however, this will clear all defined variables etc.
  // so if your intention is to maintain the state during multiple calls,
  // don't call this until you're `truly` done.
}
```

## More examples of integration
> For a better understanding, or for more examples; have a look [here](https://github.com/sebbekarlsson/glms/tree/master/src/modules).  
> In these files you can see how types, functions, globals, etc are currently implemented into the language.  
> The same principles can be applied when integrating this language in any other project.
