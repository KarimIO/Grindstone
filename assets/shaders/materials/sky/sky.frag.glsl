#version 330 core

out vec4 output;

in vec3 fragPosition;

uniform samplerCube environmentMap;

void main() {
  vec3 uv = normalize(fragPosition);
  vec4 color = texture(environmentMap, uv);
	output = vec4(color.rgb, 1);
}
