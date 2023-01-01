number w = 640;
number h = 480;

image img = image.make(w, h);

img.shade((vec3 uv, vec3 fragCoord, vec3 resolution) => {
  vec3 center = resolution * 0.5;
  number d = abs(distance(fragCoord, center));
  number g = 255 * (d < TAU * 6.0 * (1.0 + random()));
  vec3 color = mix(vec3(0.1, 0.3, 0.9), vec3(1), g);
  return vec4(color.xyz, 1.0);
});

img.save("test2.png");
