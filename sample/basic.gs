const noise = (vec3 uv) => {
  return fract(cos(uv.x * 2.192 + uv.y) * dot(uv, vec3(1.1245, 0.99181, 0.0)));
};


for (number i = 0; i < 3; i++) {

   print(noise(vec3(12, 12, 4)));
 }
