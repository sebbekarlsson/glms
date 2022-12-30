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
