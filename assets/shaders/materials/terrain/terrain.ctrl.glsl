#version 410 core

layout(vertices = 3) out;

in vec2 ctrlTexCoord[];
//out vec2 evalTexCoord;[];

#define ID gl_InvocationID

void main()
{
    if (ID == 0) {
        gl_TessLevelInner[0] = 1;
        gl_TessLevelOuter[0] = 1;
        gl_TessLevelOuter[1] = 1;
        gl_TessLevelOuter[2] = 1;
    }
}