#name Screen Space Reflections
#renderQueue Lighting
#shaderModule compute
#version 450

layout(std140, binding = 0) uniform EngineUbo {
	mat4 proj;
	mat4 view;
	vec4 eyePos;
	vec2 framebufferResolution;
	vec2 renderResolution;
	vec2 renderScale;
	float time;
} ubo;

layout(binding = 1, rgba16f) restrict writeonly uniform image2D outImage;
layout(binding = 2) uniform sampler2D colorImage;
layout(binding = 3) uniform sampler2D gbufferDepth;
layout(binding = 4) uniform sampler2D gbufferNormal;
layout(binding = 5) uniform sampler2D gbufferSpecularRoughness;

vec3 ViewNormal(vec3 inNorm) {
	return (inverse(ubo.view) * vec4(normalize(inNorm), 0)).xyz;
}

vec4 ComputeClipSpacePosition(vec2 positionNDC, float deviceDepth) {
	vec4 positionCS = vec4(positionNDC * 2.0 - 1.0, deviceDepth, 1.0);

	return positionCS;
}

vec4 ComputeViewSpacePosition(vec2 positionNDC, float deviceDepth) {
	vec4 positionCS  = ComputeClipSpacePosition(positionNDC, deviceDepth);
	vec4 positionVS = inverse(ubo.proj) * positionCS;
	positionVS.xyz /= positionVS.w;
	return positionVS;
}

vec4 GetPosition(vec2 texCoords) {
	float depthFromTexture = texture(gbufferDepth, texCoords * ubo.renderScale).x;
	vec4 position = ComputeViewSpacePosition(texCoords, depthFromTexture);
	return position;
}

vec4 ViewSpaceToScreenSpace(vec4 worldPosition, vec2 pixelSize) {
	vec4 projectedCoords = ubo.proj * worldPosition;
	projectedCoords.xyz /= projectedCoords.w;
	projectedCoords.xy = (projectedCoords.xy * 0.5f + 0.5f) * pixelSize;
	return projectedCoords;
}

vec3 GetNormal(vec2 texCoords) {
	return normalize(ViewNormal(texture(gbufferNormal, texCoords).rgb));
}

const float maxDistance = 15;
const float resolution  = 0.3;
const int   maxSteps    = 5;
const float thickness   = 0.5;

vec4 ReflectRay(vec4 positionFrom, vec3 pivot) {
	vec2 pixelSize = ubo.renderResolution;

	vec4 startPosViewSpace = vec4(positionFrom.xyz + (pivot *         0.0), 1.0);
	vec4 endPosViewSpace   = vec4(positionFrom.xyz + (pivot * maxDistance), 1.0);
	vec4 positionTo = positionFrom;

	vec4 startPosPixelSpace = ViewSpaceToScreenSpace(startPosViewSpace, pixelSize);
	vec2 currPosPixelSpace = startPosPixelSpace.xy;
	
	vec2 uv = currPosPixelSpace / pixelSize;
	vec4 endPosPixelSpace = ViewSpaceToScreenSpace(endPosViewSpace, pixelSize);
	
	float searchStart = 0;
	float searchEnd = 0;
	
	vec2 delta = endPosPixelSpace.xy - startPosPixelSpace.xy;
	float useX = abs(delta.x) >= abs(delta.y) ? 1.0 : 0.0;
	float deltaMax = mix(abs(delta.y), abs(delta.x), useX) * clamp(resolution, 0.0, 1.0);
	vec2 increment = vec2(delta.x, delta.y) / max(deltaMax, 0.001);

	int hitInitialPass = 0;
	int hitBinarySearchPass = 0;

    float viewDistance = startPosViewSpace.z;
	float deltaDepth = thickness;

    int i = 0;
	for (i = 0; i < int(deltaMax); ++i) {
		currPosPixelSpace += increment;
		uv = currPosPixelSpace.xy / pixelSize;
		positionTo = GetPosition(uv);

		searchEnd =
			mix(
				(currPosPixelSpace.y - startPosPixelSpace.y) / delta.y,
				(currPosPixelSpace.x - startPosPixelSpace.x) / delta.x,
				useX
			);

		searchEnd = clamp(searchEnd, 0.0, 1.0);

		viewDistance = (endPosViewSpace.z * startPosViewSpace.z) / mix(endPosViewSpace.z, startPosViewSpace.z, searchEnd);
		deltaDepth = viewDistance - positionTo.z;

		if (deltaDepth > 0 && deltaDepth < thickness) {
			hitInitialPass = 1;
			break;
		}
		else {
			searchStart = searchEnd;
		}
	}
/*
	searchEnd = searchStart + ((searchEnd - searchStart) / 2.0);
	const int steps = maxSteps * hitInitialPass;
	
	for (i = 0; i < steps; ++i) {
		currPosPixelSpace = mix(startPosPixelSpace.xy, endPosPixelSpace.xy, searchEnd);
		uv.xy = currPosPixelSpace.xy / pixelSize;
		positionTo = GetPosition(uv);

		viewDistance = (endPosViewSpace.z * startPosViewSpace.z) / mix(endPosViewSpace.z, startPosViewSpace.z, searchEnd);
		deltaDepth = viewDistance - positionTo.z;

		if (deltaDepth > 0 && deltaDepth < thickness) {
			hitBinarySearchPass = 1;
			searchEnd = searchStart + ((searchEnd - searchStart) / 2);
		} else {
			float currentEnd = searchEnd;
			searchEnd = searchEnd + ((searchEnd - searchStart) / 2);
			searchStart = currentEnd;
		}
	}
*/

	float distanceFromReflToTarget = length(positionTo - positionFrom);

	float visibility = hitInitialPass
		* positionTo.w
		* ( 1 - max( dot(-normalize(positionFrom.xyz), pivot), 0))
		* ( 1 - clamp( deltaDepth / thickness, 0, 1))
		* ( 1 - clamp(distanceFromReflToTarget / maxDistance, 0, 1))
		* ((uv.x < 0 || uv.x > 1 || uv.y < 0 || uv.y > 1) ? 0 : 1);

	visibility = clamp(visibility, 0, 1);

	return vec4(uv, 0, visibility);
}

layout(local_size_x = 1, local_size_y = 1) in;
void main() {
	ivec2 invocID = ivec2(gl_GlobalInvocationID);
	vec2 texCoords = vec2(invocID.x / ubo.framebufferResolution.x, invocID.y / ubo.framebufferResolution.y);
	
	vec4 positionViewSpace = GetPosition(texCoords / ubo.renderScale);
	vec3 normal = GetNormal(texCoords);
	vec3 reflectedRay = normalize(reflect(normalize(positionViewSpace.xyz), normal));

	vec4 specRough = texture(gbufferSpecularRoughness, texCoords);
	vec3 specular = specRough.rgb;
	float roughness = specRough.a * specRough.a;

	vec4 reflCoords = ReflectRay(positionViewSpace, reflectedRay);
	vec4 color = vec4(texture(colorImage, reflCoords.xy * ubo.renderScale).xyz * reflCoords.a, reflCoords.a);

	imageStore(outImage, invocID, color);
}
#endShaderModule
