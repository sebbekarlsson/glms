#include <glms/math.h>
#include <mif/utils.h>

float glms_smoothstep_factor(float edge0, float edge1, float x) {
  x = mif_clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  return x * x * (3 - 2 * x);
}
Vector3 glms_smoothstep_vec3_factor(Vector3 edge0, Vector3 edge1, float f) {
  float x = glms_smoothstep_factor(edge0.x, edge1.x, f);
  float y = glms_smoothstep_factor(edge0.y, edge1.y, f);
  float z = glms_smoothstep_factor(edge0.z, edge1.z, f);

  return VEC3(x, y, z);
}
Vector3 glms_smoothstep_vec3_vec3(Vector3 edge0, Vector3 edge1, Vector3 v) {
  float x = glms_smoothstep_factor(edge0.x, edge1.x, v.x);
  float y = glms_smoothstep_factor(edge0.y, edge1.y, v.y);
  float z = glms_smoothstep_factor(edge0.z, edge1.z, v.z);

  return VEC3(x, y, z);
}
