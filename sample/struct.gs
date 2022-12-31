typedef struct {
  number age = 2;
  string name;
} Person;

Person p = Person(33, "John Doe");

print(p.age);
print(p.name);
