response r = fetch("https://jsonplaceholder.typicode.com/posts");

array p = r.json();

object firstPost = p[0];

print(firstPost.title);

