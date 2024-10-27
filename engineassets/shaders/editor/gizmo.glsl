#name Gizmo Shader
#renderQueue Lighting
#shaderModule vertex
#version 460

struct Element {
	mat4 matrix;
	vec4 color;
};

#define maxObjects 500

layout(set = 0, binding = 0) uniform EngineUbo {
	Element elements[maxObjects];
} ubo;

layout(location = 0) in vec3 vertexPosition;
layout(location = 0) out vec4 fragmentColor;

void main() {
	gl_Position = ubo.elements[gl_BaseInstance].matrix * vec4(vertexPosition, 1.0);
	fragmentColor = ubo.elements[gl_BaseInstance].color;
}
#endShaderModule
#shaderModule fragment
#version 460

layout(location = 0) in vec4 fragmentColor;
layout(location = 0) out vec4 outColor;

void main() {
	outColor = fragmentColor;
}

#endShaderModule
