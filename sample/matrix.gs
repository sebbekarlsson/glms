number w = 640;
number h = 480;
number aspect = w / h;

mat4 model = identity();
mat4 view = identity();
projection = perspective(radians(60.0), aspect, 0.1, 100.0);

mat4 mvp = projection * view * model;


vec3 point = vec3(1, 2, 3);

vec4 p = mvp * vec4(point.xyz, 1);
print(p);
