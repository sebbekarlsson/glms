float some_util_function(float a, float b) {
  return a + b;
}

vec3 getLightAtt(in Light light, in vec3 p, in vec3 n) {
    vec3 L = getLightDir(light, p);
    float NdotL = max(0.0015, dot(n, L));

    switch (light.type) {
        case LIGHT_AMBIENT: return light.color * light.strength * NdotL; break;
        case LIGHT_POINT: return light.color * NdotL * (
            pow(light.strength, 2.0) / max(0.0001, pow(distance(light.position, p), 2.0))
        ); break;
        case LIGHT_DIR: return light.color * light.strength * NdotL * smoothstep(0.8, 1.0, dot(
            normalize(light.position - p),
            L
        )); break;
    }
    
    return vec3(0.0);
}
