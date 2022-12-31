## Examples

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
