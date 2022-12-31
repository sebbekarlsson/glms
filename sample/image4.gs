number w = 640;
number h = 480;

image img = image.make(w, h);


function noise(vec3 uv) {
  return fract(cos(uv.x * 2.192 + uv.y) * dot(uv, vec3(1.1245, 0.99181, 0.0)));
}


img.shade((vec3 uv, vec3 fragCoord, vec3 resolution) => {
  number g = noise(uv * 10.0) * 255;
  return vec4(g, g, g, 255.0);
});

img.save("test.png");
