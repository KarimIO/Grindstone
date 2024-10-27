#name Shadow Mapping
#cullMode None
#renderQueue Lighting
#shaderModule vertex
#version 450

layout(location = 0) in vec3 vertexPosition;

layout(binding = 0) uniform LightUbo {
	mat4 shadowMatrix;
} light;

void main() {
	gl_Position = light.shadowMatrix * vec4(vertexPosition, 1.0);
}
#endShaderModule
#shaderModule fragment
#version 450

void main() {}
#endShaderModule
