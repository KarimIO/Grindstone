#version 330 core

out vec4 outColor;

in vec3 fragPosition;
in vec3 fragNormal;
in vec3 fragTangent;
in vec2 fragTexCoord;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;


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
	vec4 albedo = texture(albedoTexture, fragTexCoord);
	if (albedo.a < 0.9) discard;
	vec3 normal = gl_FrontFacing ? fragNormal : -fragNormal;
	vec3 tangent = gl_FrontFacing ? fragTangent : -fragTangent;
	vec3 nt = texture(normalTexture, fragTexCoord).rgb;
	vec3 n = CalcBumpedNormal(normal, nt, tangent);

	vec3 l = vec3(-1,-1,-1);
	float NDotL = max(dot(n,l), 0.0);
	outColor = vec4(albedo.rgb * (NDotL * 0.8f + 0.2f),1);
}
