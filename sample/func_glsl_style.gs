import "func2.gs" as helpers;
import "func2.gs" as helpers;
import "func2.gs" as helpers;
import "func2.gs" as helpers;
import "func2.gs" as helpers;

glsl`
layout (location = 0) in vec3 attr_vertex;
layout (location = 1) in vec2 attr_uv;
layout (location = 2) in vec3 attr_normal;
`

uniform sampler2D myTexture;

typedef struct {
  int on;
  int id;
  int type;
  float strength;
  float radius;
  vec3 position;
  vec3 direction;
  vec4 color;
} WELLLight;

glsl`
layout (std430) readonly buffer storage_lights
{
  WELLLight s_lights[];
};
`
void hello() {
  add(5.0281, 4);
}

typedef struct {
  vec3 p;
  vec3 n;
} Data;


for (int i = 0; i < 10; i++) {
  print(i);
}


void main() {
  vec3 col = texture(myTexture, uv);

  vec3 normal = mat3(transpose(inverse(model))) * attr_normal;
}
