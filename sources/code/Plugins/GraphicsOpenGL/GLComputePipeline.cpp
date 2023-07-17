#include <GL/gl3w.h>
#include "GLUniformBuffer.hpp"
#include "GLTexture.hpp"
#include <vector>
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "GLComputePipeline.hpp"
#include "GLCore.hpp"
#include <cstring>

using namespace Grindstone::GraphicsAPI;

GLComputePipeline::GLComputePipeline(ComputePipeline::CreateInfo& createInfo) {
	CreatePipeline(createInfo);
}

void GLComputePipeline::Recreate(CreateInfo& createInfo) {
	glUseProgram(0);
	glDeleteProgram(program);

	CreatePipeline(createInfo);
}

void GLComputePipeline::CreatePipeline(CreateInfo& createInfo) {
	program = glCreateProgram();
	glObjectLabel(GL_PROGRAM, program, -1, createInfo.debugName);

	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	{
		if (createInfo.shaderFileName != nullptr) {
			glObjectLabel(GL_SHADER, shader, -1, createInfo.shaderFileName);
		}
		glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, createInfo.shaderContent, createInfo.shaderSize);
		glSpecializeShader(shader, "main", 0, 0, 0);
		glAttachShader(program, shader);
	}

	GLint result = 0;
	glLinkProgram(program);

	GLint isLinked;
	glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE) {
		GLsizei infoLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLength);
		std::vector<char> programLinkErrorMessage(infoLength + 1);
		glGetProgramInfoLog(program, infoLength, NULL, programLinkErrorMessage.data());
		printf("%s\n", programLinkErrorMessage.data());
	}

	glDeleteShader(shader);
}

void GLComputePipeline::Bind() {
	glUseProgram(program);
}

GLComputePipeline::~GLComputePipeline() {
	glUseProgram(0);
	glDeleteProgram(program);
}
