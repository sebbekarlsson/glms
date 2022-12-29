number w = 300;
number h = 300;

image img = image.make(w, h);

for (number x = 0; x < w; x++) {
  for (number y = 0; y < h; y++) {
    if (random(0, 1) >= 0.5) {
      img.setPixel(x, y, vec4(255, 0, 0, 255));
    } else {
      img.setPixel(x, y, vec4(0, 0, 0, 255));
    }
  }
}

img.save("test.png");
