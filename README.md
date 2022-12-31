<div align="center" style="text-align: center;">
  <img width="200" src="glms.png"/>
</div>

### Generalized Linear Model Script
> A scripting language focused on linear algebra, heavily inspired by [GLSL](https://en.wikipedia.org/wiki/OpenGL_Shading_Language)
> and [Javascript](https://en.wikipedia.org/wiki/JavaScript).

> This language comes with types, functions and structures commonly used when doing linear algebra,
> such as: `vec2`, `vec3`, `vec4`, `dot`, `cross`, `lerp`, `sin`, `cos` ...etc

> (There are more to come, and everything is not implemented yet).

### Caution
> This is a work in progress!

### Building & using it
```bash
mkdir build
cd build
cmake .. && make -j8

./glms_e <input_file.gs>
```

### Built-in functions
* `dot(vec a, vec b)`
* `distance(vec a, vec b)`
* `cross(vec a, vec b)`
* `normalize(vec a)`
* `unit(vec a)` - (alias for `normalize`)
* `length(any a)` - (can take vectors, strings and arrays) 
* `cos(number a)`
* `sin(number a)`
* `tan(number a)`
* `fract(number a)`
* `abs(number a)`
* `atan(number a)`
* `atan(number a, number b)`
* `lerp(number from, number to, number scale)`
* `mix(number from, number to, number scale)` - (alias for `lerp`)
* `clamp(number value, number min, number max)`
* `min(number a)`
* `max(number a)`
* `pow(number a, number b)`
* `log(number a)`
* `log10(number a)`
* `random()`
* `random(number min, number max)`
* `random(number min, number max, number seed)`

## Some examples

### Shader-like image manipulation
```
number w = 512;
number h = 512;

image img = image.make(w, h);

img.shade((vec3 uv, vec3 fragCoord, vec3 resolution) => {
  vec3 center = resolution * 0.5;
  number d = abs(distance(fragCoord, center));
  number g = 255 * (d < TAU * 6.0);
  return vec4(g, g, g, 255.0);
});

img.save("test.png");
```

### Functional programming
```
array arr = [1, 2, 3];

array mapped = arr.map((number v) => v*2);

print(mapped);  // [2.000000, 4.000000, 6.000000]
```

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

number dp = dot(d, f);

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

## Syntax highlighting
* [glms-mode for emacs](https://github.com/sebbekarlsson/glms-mode)
