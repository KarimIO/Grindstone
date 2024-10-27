#name Depth of Field (Blur)
#renderQueue Lighting
#shaderModule vertex
#version 450

layout(location = 0) in vec2 vertexPosition;

layout(location = 0) out vec2 fragmentTexCoord;
layout(location = 1) out vec2 scaledFragmentTexCoord;

layout(std140, binding = 0) uniform EngineUbo {
	mat4 proj;
	mat4 view;
	vec4 eyePos;
	vec2 framebufferResolution;
	vec2 renderResolution;
	vec2 renderScale;
	float time;
} ubo;

void main() {
	gl_Position = vec4(vertexPosition, 0.0, 1.0);
	fragmentTexCoord = ((vertexPosition * 0.5f) + vec2(0.5f));
	scaledFragmentTexCoord = fragmentTexCoord * ubo.renderScale * 4; 
}
#endShaderModule
#shaderModule fragment
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

layout(set = 1, binding = 0) uniform sampler2D unblurredTexture;

layout(location = 0) in vec2 fragmentTexCoord;
layout(location = 1) in vec2 scaledFragmentTexCoord;
layout(location = 0) out vec4 outBlurredTexture;

vec2 unitSquareToUnitDiskPolar(vec2 inCoord)
{
    float radius;
    float angle;
    if (abs(inCoord.x) > abs(inCoord.y))
    {
        radius = inCoord.x;
        angle = (inCoord.y / (inCoord.x + 9.9999999747524270787835121154785e-07)) * 0.785398006439208984375;
    }
    else
    {
        radius = inCoord.y;
        angle = 1.5707962512969970703125 - ((inCoord.x / (inCoord.y + 9.9999999747524270787835121154785e-07)) * 0.785398006439208984375);
    }
    if (radius < 0.0)
    {
        radius *= (-1.0);
        angle += 3.141592502593994140625;
    }
    return vec2(radius, angle);
}

vec2 squareToPolygonMapping(vec2 uv, float edgeCount, float shapeRotation)
{
    vec2 param = uv;
    vec2 polarCoord = unitSquareToUnitDiskPolar(param);
    polarCoord.x *= (cos(3.141592502593994140625 / edgeCount) / cos(polarCoord.y - ((6.28318500518798828125 / edgeCount) * floor((((edgeCount * polarCoord.y) + 3.141592502593994140625) / 2.0) / 3.141592502593994140625))));
    polarCoord.y += shapeRotation;
    return vec2(polarCoord.x * cos(polarCoord.y), polarCoord.x * sin(polarCoord.y));
}

void main()
{
    vec4 unblurred = texture(unblurredTexture, scaledFragmentTexCoord);
    
    float coc01 = unblurred.w;
    if (coc01 == 0.0)
    {
        discard;
    }
    
    float cocPixels = (coc01 * 32.0) / ubo.renderResolution.x;
    vec3 blurredColor = vec3(0.0);
    float numTapsInAxisFloat = 4.0;
    for (uint x = 0u; x < 4u; x++)
    {
        for (uint y = 0u; y < 4u; y++)
        {
            vec2 squareCoord = vec2(float(x) / numTapsInAxisFloat, float(y) / numTapsInAxisFloat);
            vec2 param = squareCoord;
            float param_1 = 6.0;
            float param_2 = 0.0;
            vec2 sampleCoord = squareToPolygonMapping(param, param_1, param_2);
            blurredColor += texture(unblurredTexture, scaledFragmentTexCoord + (sampleCoord * cocPixels)).xyz;
        }
    }
    outBlurredTexture = vec4(blurredColor / 16.0, coc01);
}
#endShaderModule
