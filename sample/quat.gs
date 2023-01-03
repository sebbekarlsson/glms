vec4 q = quatFor(vec3(-1, 0, 0), vec3(0, 1, 0));


vec3 p = vec3(1);

vec4 n = vec4(p, 1.0) * q;
