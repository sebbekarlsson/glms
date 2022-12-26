const x = {
  "hello": 123,
  "nested": {
    "yo": 42,
    "other": {
      name: "John"
    }
  }
};


x.hello = 33;
const y = x.hello;
print(x.nested.other.name);
