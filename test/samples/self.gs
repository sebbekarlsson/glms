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

string x = p.name;
