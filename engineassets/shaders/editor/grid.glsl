#name Grid Shader
#renderQueue Lighting
#shaderModule vertex
#version 450

vec3 gridVertices[6] = vec3[](
	vec3( 1,  1, 0),
	vec3(-1, -1, 0),
	vec3(-1,  1, 0),
	vec3(-1, -1, 0),
	vec3( 1,  1, 0),
	vec3( 1, -1, 0)
);

layout(location = 0) out vec3 nearPoint;
layout(location = 1) out vec3 farPoint;

layout(std140, binding = 0) uniform GridUbo {
	mat4 projMatrix;
	mat4 viewMatrix;
	vec4 colorXAxis;
	vec4 colorZAxis;
	vec4 colorMinor;
	vec4 colorMajor;
	vec2 renderScale;
	float fadeDistanceMultiplier;
	float nearDistance;
	float farDistance;
} ubo;

vec3 UnprojectPoint(float x, float y, float z) {
	vec4 unprojectedPoint =  inverse(ubo.viewMatrix) * inverse(ubo.projMatrix) * vec4(x, y, z, 1.0);
	return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
	vec3 vertexPosition = gridVertices[gl_VertexIndex];
	nearPoint = UnprojectPoint(vertexPosition.x, vertexPosition.y, 0.0);
	farPoint = UnprojectPoint(vertexPosition.x, vertexPosition.y, 1.0);
	gl_Position = vec4(vertexPosition, 1.0);
}
#endShaderModule
#shaderModule fragment
#version 460

layout(location = 0) in vec3 nearPoint;
layout(location = 1) in vec3 farPoint;
layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform GridUbo {
	mat4 projMatrix;
	mat4 viewMatrix;
	vec4 colorXAxis;
	vec4 colorZAxis;
	vec4 colorMinor;
	vec4 colorMajor;
	vec2 renderScale;
	float fadeDistanceMultiplier;
	float nearDistance;
	float farDistance;
} ubo;

vec4 GetGridColor(vec3 fragPos3D, float scale, vec4 gridColor) {
	vec2 coord = fragPos3D.xz * scale;
	vec2 derivative = fwidth(coord);
	vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
	float line = min(grid.x, grid.y);
	vec2 minimum = min(derivative, 1);
	float colorFade = 1.0 - min(line, 1.0);
	
	vec4 color = gridColor;
	
	if(fragPos3D.x > -0.1 * minimum.x && fragPos3D.x < 0.1 * minimum.x)
		color = ubo.colorXAxis;
		
	if(fragPos3D.z > -0.1 * minimum.y && fragPos3D.z < 0.1 * minimum.y)
		color = ubo.colorZAxis;

	color.a *= colorFade;

	return color;
}

float ComputeDepth(vec3 pos) {
	vec4 clipSpacePos = ubo.projMatrix * ubo.viewMatrix * vec4(pos.xyz, 1.0);
	return (clipSpacePos.z / clipSpacePos.w);
}

float ComputeLinearDepth(float depth) {
	float linearDepth = (2.0 * ubo.nearDistance * ubo.farDistance) / (ubo.farDistance + ubo.nearDistance - depth * (ubo.farDistance - ubo.nearDistance));
	return linearDepth / ubo.farDistance;
}

void main() {
	float t = -nearPoint.y / (farPoint.y - nearPoint.y);
	vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);
	float depth = ComputeDepth(fragPos3D);
	gl_FragDepth = depth;

	float linearDepth = ComputeLinearDepth(depth);
	float fading = clamp(1 - linearDepth, 0, 1);

	vec4 gridColor = (GetGridColor(fragPos3D, 10, ubo.colorMinor) + GetGridColor(fragPos3D, 1, ubo.colorMajor)) / 2;
	outColor = gridColor * float(t > 0);
	outColor.a *= fading;
}

#endShaderModule
