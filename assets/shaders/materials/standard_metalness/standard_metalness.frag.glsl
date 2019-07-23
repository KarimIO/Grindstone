#version 330 core

layout(location = 0) out vec4 out0;
layout(location = 1) out vec4 out1;
layout(location = 2) out vec4 out2;

in vec3 fragPosition;
in vec3 fragNormal;
in vec3 fragTangent;
in vec2 fragTexCoord;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D metalnessTexture;

layout(std140) uniform Parameters {
  bool hasAlbedoTexture;
  bool hasNormalTexture;
  bool hasRoughTexture;
  bool hasMetalTexture;
	vec4 albedoConstant;
  float metalnessConstant;
  float roughnessConstant;
} param;

vec3 CalcBumpedNormal(vec3 Ng, vec3 Nt, vec3 Tan) {
  vec3 BumpMapNormal = Nt;
  if (BumpMapNormal == vec3(0))
  return Ng;
  vec3 NewNormal = normalize(Ng);
  vec3 NewTangent = normalize(Tan);
  NewTangent = normalize(NewTangent - dot(NewTangent, NewNormal) * NewNormal);
  vec3 Bitangent = cross(NewTangent, NewNormal);
  BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
  BumpMapNormal = vec3(-BumpMapNormal.xy, BumpMapNormal.z);
  mat3 TBN = mat3(NewTangent, Bitangent, NewNormal);
  return normalize(TBN * BumpMapNormal);
}

void main() {
	vec4 albedo     = param.albedoConstant * (param.hasAlbedoTexture ? texture(albedoTexture, fragTexCoord) : vec4(1));
	float roughness = param.hasRoughTexture ? texture(roughnessTexture, fragTexCoord).r : param.roughnessConstant;
	float metalness = param.hasMetalTexture ? texture(metalnessTexture, fragTexCoord).r : param.metalnessConstant;

	out0 = (1 - metalness) * albedo;
	out0 = pow(out0, vec4(2.2f));
	if (false && param.hasNormalTexture) {
		out1 = vec4(texture(normalTexture, fragTexCoord).rgb, 1);
		out1 = vec4(CalcBumpedNormal(fragNormal, out1.rgb, fragTangent), 1);
	}
	else {
		out1 = vec4(fragNormal, 1);
	}

	vec3 Specular = mix(vec3(0.04), albedo.rgb, metalness);
	out2 = vec4(Specular, roughness);
}
