# GLMS
### Generalized Linear Model Language
> A scripting language focused on linear algebra, heavily inspired by [GLSL](https://en.wikipedia.org/wiki/OpenGL_Shading_Language)
> and [Javascript](https://en.wikipedia.org/wiki/JavaScript).

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
