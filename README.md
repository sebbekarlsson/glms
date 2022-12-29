# GLMS
### Generalized Linear Model Language
> A scripting language focused on linear algebra, heavily inspired by [GLSL](https://en.wikipedia.org/wiki/OpenGL_Shading_Language)
> and [Javascript](https://en.wikipedia.org/wiki/JavaScript).

> This language comes with types, functions and structures commonly used when doing linear algebra,
> such as: `vec2`, `vec3`, `vec4`, `dot`, `cross`, `lerp`, `sin`, `cos` ...etc

> (There are more to come, and everything is not implemented yet).

### Caution
> This is a work in progress!

## Some examples

### Vectors
```
vec3 a = vec3(1, 0, 0);
vec3 b = vec3(0, 1, 0);
vec3 c = cross(a, b);

print(c);

vec3 d = vec3(random(), random(), random());

vec3 e = vec3(random(), random(), random());

print(distance(d, e));

vec3 f = vec3(random());

print(f);

dp = dot(d, f);

print(dp);
```

### Lerp
```
number x = 25.012;
number y = 98.241;

number z = lerp(x, y, 0.1);

print(z); // 32.334900
```

### Clamp
```
number value = 2312.0;
value = clamp(value, 0.0, 1.0);
print(value); // 1.000000
```

### cos, sin
```
number a = cos(1.63124);
number b = sin(3.33124);

print(a); // -0.060407
print(b); // -0.188513
```

### Arrow functions
```
const greeting = (msg) => print(msg);

greeting("hello world!"); // hello world!
```

### Structs
```
typedef struct {
  number age;
  string name;
} Person;

Person p = Person(33, "John Doe");

print(p.age); // 33.000000
print(p.name); // John Doe
```

### Loops
```
number x = 100;

while (x > 0) {
  x--;
}

print(x); // 0.000000

for (number i = 0; i < 3; i++) {
  print(i);
}

// 0.000000
// 1.000000
// 2.000000

```

### Image manipulation
```
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
```
