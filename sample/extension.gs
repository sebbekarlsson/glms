import "/home/ianertson/workspace/glms-canvas/build/libglms_canvas.so" as canvasLib;
typedef (canvasLib.canvas) canvas; // we're making a "promise" to the interpreter
                                   // that this type will exist later.

canvas c = canvas(640, 480);

// opens a window, changes can be seen in real-time
c.shade((vec3 uv, vec3 fragCoord, vec3 resolution, number time) => {
    vec3 p = (uv * 3.912) + vec3(cos(time), sin(time), 0);
    float n = fract(cos(p.x + 1.69482 * p.y) * dot(p, vec3(1.2314 + fract(time/1.6), 1.1111, 0)));
    return vec4(vec3(n), 1.0);
    //    return vec4(0.5 * (0.5 + cos(uv.x*time)), 0.5 * (0.5 + sin(uv.y * time)), cos(time), 1.0);
});
