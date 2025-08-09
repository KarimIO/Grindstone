#include <GL/gl3w.h>
#include <Grindstone.RHI.OpenGL/include/GLBuffer.hpp>
#include <vector>
#include <iostream>
#include <cstring>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <Grindstone.RHI.OpenGL/include/GLComputePipeline.hpp>
#include <Grindstone.RHI.OpenGL/include/GLCore.hpp>

using namespace Grindstone::GraphicsAPI;

OpenGL::ComputePipeline::ComputePipeline(const ComputePipeline::CreateInfo& createInfo) {
	CreatePipeline(createInfo);
}

void OpenGL::ComputePipeline::Recreate(const ComputePipeline::CreateInfo& createInfo) {
	glUseProgram(0);
	glDeleteProgram(program);

	CreatePipeline(createInfo);
}

void OpenGL::ComputePipeline::CreatePipeline(const ComputePipeline::CreateInfo& createInfo) {
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

void OpenGL::ComputePipeline::Bind() {
	glUseProgram(program);
}

OpenGL::ComputePipeline::~ComputePipeline() {
	glUseProgram(0);
	glDeleteProgram(program);
}
