file f = file.open("assets/somefile.json", "r");
string contents = f.read();
f.close();

object data = json.parse(contents);
  
print(data.firstName);
print(data.age);
