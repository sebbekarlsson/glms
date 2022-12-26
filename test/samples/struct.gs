typedef struct {
  number age;
  string name;
} Person;

Person p;

p.age = 33;
p.name = "John doe";
print(p);

Person p2;
p2.age = 22;
p2.name = "Sarah Doe";
print(p2);

g = struct {
  string name = "test";
};

print(g);
