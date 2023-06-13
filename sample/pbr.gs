import "material.gs" as i_material;
fdecl PBRMaterial;

void get_pbr_from_gbuffer(inout PBRMaterial m, in vec3 normal) {
  //  pbr_material_init(m);


  float AO = 0.0;
  float roughness = 0.0;
  float metallic = 0.0;

  if (u_material.arm > 0) {
    vec4 i_arm =  texture(sampler2D(u_material.arm), uv);
    AO = i_arm.x;
    roughness = i_arm.y;
    metallic = i_arm.z;
  } else if (u_material.rough > 0) {
    vec4 i_rough =  texture(sampler2D(u_material.rough), uv);
    AO = i_rough.x;
    roughness = i_rough.y;
    metallic = i_rough.z;
  } else if (u_material.AO > 0) {
    vec4 i_AO = texture(sampler2D(u_material.AO), uv);
    AO = i_AO.x;
  }
  
  vec3 n = normalize(normal);
  vec4 i_bump = texture(sampler2D(u_material.normal), uv);
  vec4 i_albedo = texture(sampler2D(u_material.albedo), uv);
  vec4 i_spec = texture(sampler2D(u_material.spec), uv);



  m.AO = AO;
  m.roughness = roughness;
  m.metallic = metallic;
  m.spec = i_spec.rgb;
  m.normal = normalize(n + 0.25*normalize(i_bump.xyz));
  m.albedo = i_albedo;
}
