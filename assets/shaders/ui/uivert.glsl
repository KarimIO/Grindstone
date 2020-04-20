#version 330 core
layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec2 vertexCoord;

void main() {
    gl_Position = vec4(vertexPosition, 0, 1);
	//fragTexCoords = vertexCoord;
}
