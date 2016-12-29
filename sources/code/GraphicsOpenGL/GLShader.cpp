#include "gl3w.h"
#include "GLShader.h"
#include <vector>
#include <iostream>
#include <glm/glm.hpp>

GLShaderProgram::GLShaderProgram()
{
	program = glCreateProgram();
	shaderNum = 0;
}

bool GLShaderProgram::AddShader(std::string path, std::string content, ShaderType type) {

	// Compile Vertex Shader
	int shaderType = GL_VERTEX_SHADER;
	if (type == SHADER_FRAGMENT)
		shaderType = GL_FRAGMENT_SHADER;

	shaders[shaderNum] = glCreateShader(shaderType);

	const GLchar* str[1];
	str[0] = content.c_str();
	GLint length[1] = { (GLint)content.size() };

	glShaderSource(shaders[shaderNum], 1, str, length);
	glCompileShader(shaders[shaderNum]);

	GLint result = GL_FALSE;
	int infoLength;
	GLint isCompiled = 0;

	// Check Vertex Shader
	glGetShaderiv(shaders[shaderNum], GL_COMPILE_STATUS, &result);
	glGetShaderiv(shaders[shaderNum], GL_INFO_LOG_LENGTH, &infoLength);
	glGetShaderiv(shaders[shaderNum], GL_COMPILE_STATUS, &isCompiled);
	if (!isCompiled) {
		printf("Error Report Vertex Shader %s\n", path.c_str());
		std::vector<char> VertexShaderErrorMessage(infoLength + 1);
		glGetShaderInfoLog(shaders[shaderNum], infoLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
		return false;
	}

	glAttachShader(program, shaders[shaderNum++]);

	return true;
}

bool GLShaderProgram::Compile() {
	GLint result = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if (!result) {
		int maxLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		return false;
	}

	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
	if (!result) {
		int maxLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

		glGetProgramInfoLog(program, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		return false;
	}

	// Detach Shaders (Remove their references)
	glDetachShader(program, shaders[0]);
	glDetachShader(program, shaders[1]);

	// Cleanup
	glDeleteShader(shaders[0]);
	glDeleteShader(shaders[1]);

	return true;
}

void GLShaderProgram::Use() {
	glUseProgram(program);
}

void GLShaderProgram::SetNumUniforms(int num) {
	uniforms = new int[num];
	uniformCounter = 0;
	dataOffset = 0;
}

void GLShaderProgram::CreateUniform(const char *name) {
	uniforms[uniformCounter++] = glGetUniformLocation(program, name);
}

void GLShaderProgram::PassData(void *ptr) {
	dataPtr = (char*)ptr;
	uniformCounter = 0;
	dataOffset = 0;
}

void GLShaderProgram::SetUniform4m() {
	glm::mat4 data = *(glm::mat4 *)(dataPtr);
	glUniformMatrix4fv(uniforms[uniformCounter++], 1, GL_FALSE, &data[0][0]);
	dataPtr += sizeof(glm::mat4);
}

void GLShaderProgram::SetInteger() {
	int data = *(int *)(dataPtr);
	glUniform1i(uniforms[uniformCounter++], data);
	dataPtr += sizeof(int);
}

void GLShaderProgram::SetVec3() {
	glm::vec3 data = *(glm::vec3 *)(dataPtr);
	glUniform3f(uniforms[uniformCounter++], data.x, data.y, data.z);
	dataPtr += sizeof(glm::vec3);
}

void GLShaderProgram::Cleanup() {
	glUseProgram(0);
	glDeleteProgram(program);
}

GRAPHICS_EXPORT ShaderProgram* createShader() {
	return new GLShaderProgram;
}