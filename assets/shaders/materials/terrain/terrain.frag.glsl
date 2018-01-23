#version 330 core

layout(location = 0) out vec4 out0;
layout(location = 1) out vec4 out1;
layout(location = 2) out vec4 out2;

in vec3 fragPosition;
in vec3 fragNormal;
in vec3 fragTangent;
in vec2 fragTexCoord;

void main() {
  out0 = vec4(fragTexCoord, 0, 1);
  out1 = vec4(fragNormal, 1);
  out2 = vec4(0.04, 0.04, 0.04, 0.8);
}
