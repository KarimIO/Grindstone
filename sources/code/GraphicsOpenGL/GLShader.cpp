#include "gl3w.h"
#include "GLShader.h"
#include "GLUniformBuffer.h"
#include <vector>
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

void GLShaderProgram::Initialize(int numShaders) {
	program = glCreateProgram();
	shaderNum = 0;
	shaders = new uint32_t[numShaders];
}

bool GLShaderProgram::AddShader(std::string *path, std::string *content, ShaderType type) {
	// Compile Vertex Shader
	int shaderType = GL_VERTEX_SHADER;
	switch (type) {
	case SHADER_FRAGMENT:
		shaderType = GL_FRAGMENT_SHADER;
		break;
	case SHADER_TESS_EVALUATION:
		shaderType = GL_TESS_EVALUATION_SHADER;
		break;
	case SHADER_TESS_CONTROL:
		shaderType = GL_TESS_CONTROL_SHADER;
		break;
	case SHADER_GEOMETRY:
		shaderType = GL_GEOMETRY_SHADER;
		break;
	case SHADER_VERTEX:
		shaderType = GL_VERTEX_SHADER;
		break;
	}

	shaders[shaderNum] = glCreateShader(shaderType);

	const GLchar* str[1];
	str[0] = (*content).c_str();
	GLint length[1] = { (GLint)(*content).size() };

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
		printf("Error Report Vertex Shader %s\n", (*path).c_str());
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
		fprintf(stderr, "Error linking shader program: '%s'\n", &infoLog[0]);
		return false;
	}

	// Detach Shaders (Remove their references)
	for (size_t i = 0; i < shaderNum; i++) {
		glDetachShader(program, shaders[i]);
		glDeleteShader(shaders[i]);
	}

	return true;
}

void GLShaderProgram::PrepareBuffer(const char *name, UniformBuffer *buffer, unsigned int location) {
	GLuint index = glGetUniformBlockIndex(program, name);
	glUniformBlockBinding(program, index, 2);

	GLUniformBuffer *glBuffer = (GLUniformBuffer *)buffer;
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, glBuffer->block);
}

bool GLShaderProgram::Validate() {
	glValidateProgram(program);
	GLint result = 0;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
	if (!result) {
		int maxLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
		fprintf(stderr, "Invalid shader program: '%s'\n", &infoLog[0]);
		return false;
	}

	return true;
}

void GLShaderProgram::Use() {
	glUseProgram(program);
}

void GLShaderProgram::BindAttribLocation(uint32_t index, const char *name) {
	glBindAttribLocation(program, index, name);

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << err << std::endl;
	}
}

void GLShaderProgram::BindOutputLocation(uint32_t index, const char *name) {
	glBindFragDataLocation(program, index, name);

	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "OpenGL error: " << err << std::endl;
	}
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

void GLShaderProgram::SetUniformFloat() {
	float data = *(float *)(dataPtr);
	glUniform1f(uniforms[uniformCounter++], data);
	dataPtr += sizeof(float);
}

void GLShaderProgram::SetInteger() {
	int data = *(int *)(dataPtr);
	glUniform1i(uniforms[uniformCounter++], data);
	dataPtr += sizeof(int);
}

void GLShaderProgram::SetFloatArray(unsigned int size) {
	glUniform3fv(uniforms[uniformCounter++], size, (GLfloat *)dataPtr);
	dataPtr += sizeof(GLfloat)*size;
}

void GLShaderProgram::SetVec4() {
	glm::vec4 data = *(glm::vec4 *)(dataPtr);
	glUniform4f(uniforms[uniformCounter++], data.x, data.y, data.z, data.w);
	dataPtr += sizeof(glm::vec4);
}

void GLShaderProgram::SetVec3() {
	glm::vec3 data = *(glm::vec3 *)(dataPtr);
	glUniform3f(uniforms[uniformCounter++], data.x, data.y, data.z);
	dataPtr += sizeof(glm::vec3);
}

void GLShaderProgram::SetVec2() {
	glm::vec2 data = *(glm::vec2 *)(dataPtr);
	glUniform2f(uniforms[uniformCounter++], data.x, data.y);
	dataPtr += sizeof(glm::vec2);
}

void GLShaderProgram::Cleanup() {
	glUseProgram(0);
	glDeleteProgram(program);
}

GRAPHICS_EXPORT ShaderProgram* createShader() {
	return new GLShaderProgram;
}