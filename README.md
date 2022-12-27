# GLMS
### Generalized Linear Model Language
> A scripting language focused on linear algebra, heavily inspired by [GLSL](https://en.wikipedia.org/wiki/OpenGL_Shading_Language)
> and [Javascript](https://en.wikipedia.org/wiki/JavaScript).

> This language comes with types, functions and structures commonly used when doing linear algebra,
> such as: `vec2`, `vec3`, `vec4`, `dot`, `cross`, `sin`, `cos` ...etc

> (There are more to come, and everything is not implemented yet).

### Caution
> This is a work in progress!

## Some examples

### Dot product
```
vec3 a = vec3(0, 1, 0);
vec3 b = vec3(0, 0.2, 0.8);
number d = dot(a, b);

print(d); // 0.200000
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