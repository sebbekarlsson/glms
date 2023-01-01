## Global functions

### unit
```
GLMS_AST_TYPE_VEC3 unit(GLMS_AST_TYPE_VEC3)

```

### transpose
```
GLMS_AST_TYPE_MAT4 transpose(GLMS_AST_TYPE_MAT4)

```

### clamp
```
GLMS_AST_TYPE_NUMBER clamp(GLMS_AST_TYPE_NUMBER value, GLMS_AST_TYPE_NUMBER min, GLMS_AST_TYPE_NUMBER max)

```

### perspective
```
GLMS_AST_TYPE_MAT4 perspective(GLMS_AST_TYPE_NUMBER fov, GLMS_AST_TYPE_NUMBER aspect, GLMS_AST_TYPE_NUMBER near, GLMS_AST_TYPE_NUMBER far)

```

### cross
```
GLMS_AST_TYPE_VEC3 cross(GLMS_AST_TYPE_VEC3, GLMS_AST_TYPE_VEC3)

```

### atan
```
GLMS_AST_TYPE_NUMBER atan(GLMS_AST_TYPE_NUMBER)
GLMS_AST_TYPE_NUMBER atan(GLMS_AST_TYPE_NUMBER, GLMS_AST_TYPE_NUMBER)

```

### log
```
GLMS_AST_TYPE_NUMBER log(GLMS_AST_TYPE_NUMBER)

```

### fract
```
GLMS_AST_TYPE_NUMBER fract(GLMS_AST_TYPE_NUMBER)

```

### dot
```
GLMS_AST_TYPE_NUMBER dot(GLMS_AST_TYPE_VEC3, GLMS_AST_TYPE_VEC3)

```

### min
```
GLMS_AST_TYPE_NUMBER min(GLMS_AST_TYPE_NUMBER, GLMS_AST_TYPE_NUMBER)

```

### sin
```
GLMS_AST_TYPE_NUMBER sin(GLMS_AST_TYPE_NUMBER)

```

### pow
```
GLMS_AST_TYPE_NUMBER pow(GLMS_AST_TYPE_NUMBER, GLMS_AST_TYPE_NUMBER)

```

### mix
```
GLMS_AST_TYPE_NUMBER mix(GLMS_AST_TYPE_NUMBER from, GLMS_AST_TYPE_NUMBER to, GLMS_AST_TYPE_NUMBER scale)
GLMS_AST_TYPE_VEC3 mix(GLMS_AST_TYPE_VEC3 from, GLMS_AST_TYPE_VEC3 to, GLMS_AST_TYPE_NUMBER scale)

```

### max
```
GLMS_AST_TYPE_NUMBER max(GLMS_AST_TYPE_NUMBER, GLMS_AST_TYPE_NUMBER)

```

### log10
```
GLMS_AST_TYPE_NUMBER log10(GLMS_AST_TYPE_NUMBER)

```

### radians
```
GLMS_AST_TYPE_NUMBER radians(GLMS_AST_TYPE_NUMBER)

```

### abs
```
GLMS_AST_TYPE_NUMBER abs(GLMS_AST_TYPE_NUMBER)

```

### identity
```
GLMS_AST_TYPE_MAT4 identity()

```

### random
```
GLMS_AST_TYPE_NUMBER random() // Returns a random value between 0 and 1.

GLMS_AST_TYPE_NUMBER random(GLMS_AST_TYPE_NUMBER min, GLMS_AST_TYPE_NUMBER max)
GLMS_AST_TYPE_NUMBER random(GLMS_AST_TYPE_NUMBER min, GLMS_AST_TYPE_NUMBER max, GLMS_AST_TYPE_NUMBER seed)

```

### cos
```
GLMS_AST_TYPE_NUMBER cos(GLMS_AST_TYPE_NUMBER)

```

### tan
```
GLMS_AST_TYPE_NUMBER tan(GLMS_AST_TYPE_NUMBER)

```

### normalize
```
GLMS_AST_TYPE_VEC3 normalize(GLMS_AST_TYPE_VEC3)

```

### distance
```
GLMS_AST_TYPE_NUMBER distance(GLMS_AST_TYPE_VEC3, GLMS_AST_TYPE_VEC3)

```

### length
```
GLMS_AST_TYPE_NUMBER length(GLMS_AST_TYPE_VEC3)
GLMS_AST_TYPE_NUMBER length(GLMS_AST_TYPE_STRING)
GLMS_AST_TYPE_NUMBER length(GLMS_AST_TYPE_ARRAY)

```

### print
> No signatures defined.


### inverse
```
GLMS_AST_TYPE_MAT4 inverse(GLMS_AST_TYPE_MAT4)

```

### lerp
```
GLMS_AST_TYPE_NUMBER lerp(GLMS_AST_TYPE_NUMBER from, GLMS_AST_TYPE_NUMBER to, GLMS_AST_TYPE_NUMBER scale)
GLMS_AST_TYPE_VEC3 lerp(GLMS_AST_TYPE_VEC3 from, GLMS_AST_TYPE_VEC3 to, GLMS_AST_TYPE_NUMBER scale)

```

### ortho
```
GLMS_AST_TYPE_MAT4 ortho(GLMS_AST_TYPE_NUMBER left, GLMS_AST_TYPE_NUMBER right, GLMS_AST_TYPE_NUMBER bottom, GLMS_AST_TYPE_NUMBER top, GLMS_AST_TYPE_NUMBER near, GLMS_AST_TYPE_NUMBER far)

```

## Types & structures

### string (struct)

### array (struct)
<details><summary>props</summary>

### array.map
> No signatures defined.



</details>


### image (struct)
<details><summary>props</summary>

### image.save
```
GLMS_AST_TYPE_BOOL image.save(GLMS_AST_TYPE_STRING filename)

```

### image.setPixel
```
GLMS_AST_TYPE_VOID image.setPixel(GLMS_AST_TYPE_VEC4 pixel)

```

### image.getPixel
```
GLMS_AST_TYPE_VEC4 image.getPixel(GLMS_AST_TYPE_NUMBER x, GLMS_AST_TYPE_NUMBER y)

```

### image.shade
```
GLMS_AST_TYPE_BOOL image.shade(GLMS_AST_TYPE_FUNC)

```

### image.make
```
image image.make(GLMS_AST_TYPE_NUMBER width, GLMS_AST_TYPE_NUMBER height)

```


</details>


### vec3 (struct)

### vec4 (struct)

### mat4 (struct)

### file (struct)
<details><summary>props</summary>

### file.write
```
GLMS_AST_TYPE_BOOL file.write(GLMS_AST_TYPE_STRING text)

```

### file.open
```
file file.open(GLMS_AST_TYPE_STRING filename, GLMS_AST_TYPE_STRING mode)

```

### file.close
```
GLMS_AST_TYPE_BOOL file.close()

```


</details>


