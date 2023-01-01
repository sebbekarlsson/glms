number w = 640;
number h = 480;

image img = image.make(w, h);

img.shade((vec3 uv, vec3 fragCoord, resolution) => {
  vec3 center = resolution * 0.5;
  number d = abs(distance(fragCoord, center));
  number g = 255 * (d < TAU * 6.0 * (1.0 + random()));
  return vec4(g, g, g, 255.0);
});

img.save("test2.png");
