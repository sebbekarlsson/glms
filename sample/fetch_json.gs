response r = fetch("https://jsonplaceholder.typicode.com/posts");

array p = r.json();

int len = p.length();
for (int i = 0; i < len; i++) {
  object post = p[i];
  print(post.title);
}



