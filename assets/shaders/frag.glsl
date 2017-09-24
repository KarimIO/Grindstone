#version 330 core

layout(location = 0) out vec4 out0;
layout(location = 1) out vec4 out1;
layout(location = 2) out vec4 out2;

in vec3 fragPosition;
in vec3 fragNormal;
in vec3 fragTangent;
in vec2 fragTexCoord;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

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
  float Metalness = texture(tex3, fragTexCoord).r;

  out0 = (1 - Metalness) * texture(tex0, fragTexCoord);
  out1 = texture(tex1, fragTexCoord);

  vec3 Normals = CalcBumpedNormal(fragNormal, out1.rgb, fragTangent);
  out1 = vec4(normalize(Normals.rgb) , 1);
  
  vec3 Specular = mix(vec3(0.04), texture(tex0, fragTexCoord).rgb, Metalness);
  out2 = vec4(Specular, texture(tex2, fragTexCoord).r);
}
