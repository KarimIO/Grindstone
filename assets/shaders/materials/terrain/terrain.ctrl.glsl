#version 410 core


layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
    vec3 eye_pos;
} ubo;

// define the number of CPs in the output patch
layout (vertices = 3) out;

// attributes of the input CPs
in vec3 WorldPos_CS_in[];
in vec3 Normal_CS_in[];
in vec2 TexCoord_CS_in[];

// attributes of the output CPs
out vec3 WorldPos_ES_in[];
out vec3 Normal_ES_in[];
out vec2 TexCoord_ES_in[];

float GetTessLevel(float Distance0, float Distance1)
{
    float d = (Distance0 + Distance1) / 2.0;
    float s = clamp((d - 16.0f) / (512.0f - 2.0f), 0, 1);
    return pow(2, (mix(32, 0, s)));
}

void main()
{
    vec3 gEyeWorldPos = ubo.eye_pos;

    // Set the control points of the output patch
    TexCoord_ES_in[gl_InvocationID] = TexCoord_CS_in[gl_InvocationID];
    Normal_ES_in[gl_InvocationID] = Normal_CS_in[gl_InvocationID];
    WorldPos_ES_in[gl_InvocationID] = WorldPos_CS_in[gl_InvocationID];

    // Calculate the distance from the camera to the three control points
    float EyeToVertexDistance0 = distance(gEyeWorldPos, WorldPos_ES_in[0]);
    float EyeToVertexDistance1 = distance(gEyeWorldPos, WorldPos_ES_in[1]);
    float EyeToVertexDistance2 = distance(gEyeWorldPos, WorldPos_ES_in[2]);

    // Calculate the tessellation levels
    gl_TessLevelOuter[0] = GetTessLevel(EyeToVertexDistance1, EyeToVertexDistance2);
    gl_TessLevelOuter[1] = GetTessLevel(EyeToVertexDistance2, EyeToVertexDistance0);
    gl_TessLevelOuter[2] = GetTessLevel(EyeToVertexDistance0, EyeToVertexDistance1);
    gl_TessLevelInner[0] = gl_TessLevelOuter[2];
} 