vec3 a = vec3(0, 0.1, 0.2);
vec3 b = vec3(4, 5, 6);

vec3 c = smoothstep(a, b, 0.5);
vec3 d = smoothstep(a, b, vec3(random(), random(), random()));

print(c);
print(d);
