#version 330 core
layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec4 vertexColor;
layout (location = 2) in vec2 vertexTexCoord;

out vec4 fragmentColor;
out vec2 fragmentTexCoord;

void main() {
    gl_Position = vec4(vertexPosition, 0, 1);
    fragmentColor = vertexColor;
	fragmentTexCoord = vertexTexCoord;
}
