function test(vec3 color) {
  color = color * 100.0;
}

vec3 j = vec3(1);

test(j);

print(j);
