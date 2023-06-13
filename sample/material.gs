typedef struct {
  float AO;
  float roughness;
  float metallic;
  vec3 spec;
  vec3 normal;
  vec4 albedo;
} PBRMaterial;

void pbr_material_init(inout PBRMaterial m) {
  m.AO = 0.0;
  m.roughness = 0.0;
  m.metallic = 0.0;
  m.spec = vec3(0.0);
  m.normal = vec3(0.0);
  m.albedo = vec4(0.0);
}
