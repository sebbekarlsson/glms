## Examples

### Importing external files
> `helpers.gs`
```glsl
function add(number x, number y) {
  return x + y;
}
```
> `main.gs`
```glsl
import "helpers.gs" as helpers

number value = helpers.add(5, 3);

print(value);
```

### Matrix math
```glsl
number w = 640;
number h = 480;
number aspect = w / h;

mat4 model = identity();
mat4 view = identity();
projection = perspective(radians(60.0), aspect, 0.1, 100.0);

mat4 mvp = projection * view * model;

vec3 point = vec3(1, 2, 3);
vec4 p = mvp * vec4(point.xyz, 1);
```

### Writing a file
```glsl
file f = file.open("test.txt", "w+");

f.write("hello!");

f.close();
```

### Struct with function
```glsl
typedef struct {
  function bark() {
    print("Woof woof!");
  }
} Dog;

Dog dog = Dog();

dog.bark();
```

### Struct and `self` keyword
```glsl
typedef struct {
  string name;

  function changeName(string newName) {
    self.name = newName;
  }
} Person;


Person p = Person("John Doe");

print(p.name);

p.changeName("David Doe");

print(p.name);
```

### Arbitrary objects
```glsl
object x = {
  "hello": 123,
  "nested": {
    "yo": 42,
    "other": {
      name: "John"
    }
  }
};

x.hello = 33;
number y = x.hello;
print(y);
print(x.nested.other.name);
```
