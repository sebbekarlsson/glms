file f = file.open("/home/ianertson/Downloads/bible.txt", "r+");

iterator it = f.readLines();


while (string x = it.next()) {
  print(x);
}

f.close();
