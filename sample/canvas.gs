canvas c = canvas(640, 480);

c.shade((vec3 uv, vec3 fragCoord, vec3 resolution, number time) => {
    return vec4(uv.x * cos(time), uv.y * sin(time), 1.0, 1.0);
});
