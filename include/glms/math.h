#ifndef GLMS_MATH_H
#define GLMS_MATH_H
#include <mif/linear/vector2/all.h>
#include <mif/linear/vector3/all.h>
#include <mif/linear/vector4/all.h>

float glms_smoothstep_factor(float edge0, float edge1, float x);
Vector3 glms_smoothstep_vec3_factor(Vector3 edge0, Vector3 edge1, float x);
Vector3 glms_smoothstep_vec3_vec3(Vector3 edge0, Vector3 edge1, Vector3 x);

#endif
