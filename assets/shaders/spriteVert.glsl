#version 330 core
layout (location = 0) in vec2 vertexPosition; // <vec2 position, vec2 texCoords>


layout(std140) uniform SpriteUBO {
    mat4 model;
    vec4 color;
    float aspect;
    bool is_ortho;
} subo;

layout(std140) uniform UniformBufferObject {
    mat4 proj_view;
    vec3 eyepos;
    float time;
    mat4 invProj;
    mat4 invView;
	vec4 resolution;
} ubo;

out vec2 fragTexCoords;

uniform mat4 model;
uniform mat4 projection;

void main() {
    vec2 scale = vec2(
        length(subo.model[0]) / subo.aspect,
        length(subo.model[1])
    );

    vec4 billboard = (subo.model * vec4(vec3(0.0), 1.0));
    vec4 newPosition = ubo.proj_view
    * billboard
    + vec4(scale * vertexPosition.xy, 0.0, 0.0);

    gl_Position = newPosition;
	fragTexCoords = (vertexPosition.xy + vec2(1,1)) / 2.0;
    fragTexCoords.y *= -1;
}