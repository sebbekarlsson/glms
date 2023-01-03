import "/home/ianertson/workspace/glms-canvas/build/libglms_canvas.so" as canvasLib;
typedef (canvasLib.canvas) canvas; // we're making a "promise" to the interpreter
                                   // that this type will exist later.


image img = image.load("/home/ianertson/Downloads/london.jpg");

canvas c = canvas(640, 480);
// opens a window, changes can be seen in real-time
c.shade((vec3 uv, vec3 fragCoord, vec3 resolution, number time) => {
    vec4 px = img.getPixel(fragCoord.x, resolution.y - fragCoord.y);
    vec3 col = vec3(px.x / 255, px.y / 255, px.z / 255);

    vec3 col2 = vec3(atan(col.r, cos(time*uv.x)), atan(col.g, sin(time*uv.y)), atan(col.b, tan(time*(uv.x + uv.y))));

    return vec4(col2, 1.0);
    //    return vec4(0.5 * (0.5 + cos(uv.x*time)), 0.5 * (0.5 + sin(uv.y * time)), cos(time), 1.0);
});
