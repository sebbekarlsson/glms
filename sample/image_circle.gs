number w = 640;
number h = 480;

image img = image.make(w, h);

function circle(vec3 uv, vec3 pos, number rad, vec3 color) {
  number d = length(pos - uv) - rad;
  number t = clamp(d, 0.0, 1.0);
  return vec4(color, 1.0 - t);
}

img.shade((vec3 uv, vec3 fragCoord, vec3 resolution) => {
  vec3 center = resolution * 0.5;
  return circle(fragCoord, center, 0.25 * resolution.y, vec3(1, 0, 0));
});

img.save("circle.png");
