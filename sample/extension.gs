import "/home/ianertson/workspace/glms-canvas/build/libglms_canvas.so" as canvasLib;
typedef (canvasLib.canvas) canvas; // we're making a "promise" to the interpreter
                                   // that this type will exist later.

canvas c = canvas(640, 480);

// opens a window, changes can be seen in real-time
c.shade((vec3 uv, vec3 fragCoord, vec3 resolution, number time) => {
    return vec4(0.5 * (0.5 + cos(uv.x*time)), 0.5 * (0.5 + sin(uv.y * time)), cos(time), 1.0);
});
