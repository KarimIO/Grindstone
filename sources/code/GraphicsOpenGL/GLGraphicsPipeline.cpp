#include <GL/gl3w.h>
#include "GLUniformBuffer.h"
#include "GLTexture.h"
#include <vector>
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include "GLGraphicsPipeline.h"
#include <cstring>

GLuint GLGraphicsPipeline::createShaderModule(ShaderStageCreateInfo createInfo) {
	int shaderType;
	switch (createInfo.type) {
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
	default:
	case SHADER_VERTEX:
		shaderType = GL_VERTEX_SHADER;
		break;
	}

	GLuint shader = glCreateShader(shaderType);

	const GLint length = createInfo.size;

	std::string test;
	test.reserve(length);
	std::memcpy((void *)test.data(), createInfo.content, length);

	glShaderSource(shader, 1, &createInfo.content, &length);
	glCompileShader(shader);

	GLint result = GL_FALSE;
	int infoLength;

	// Check Shader
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);
	if (!result) {
		printf("Error Report in Shader %s\n", createInfo.fileName);
		std::vector<char> VertexShaderErrorMessage(infoLength + 1);
		glGetShaderInfoLog(shader, infoLength, NULL, VertexShaderErrorMessage.data());
		printf("%s\n", VertexShaderErrorMessage.data());
		return false;
	}

	glAttachShader(program, shader);
	
	return shader;
}

GLGraphicsPipeline::GLGraphicsPipeline(GraphicsPipelineCreateInfo createInfo) {
	switch (createInfo.primitiveType) {
	case PRIM_TRIANGLES:
		primitiveType = GL_TRIANGLES;
		break;
	case PRIM_PATCHES:
		primitiveType = GL_PATCHES;
		break;
	}
	width = createInfo.width;
	height = createInfo.height;
	scissorW = createInfo.scissorW;
	scissorH = createInfo.scissorH;
	scissorX = createInfo.scissorX;
	scissorY = createInfo.scissorY;
	cullMode = createInfo.cullMode;

	program = glCreateProgram();

	uint32_t shaderNum = createInfo.shaderStageCreateInfoCount;
	GLuint *shaders = new GLuint[shaderNum];
	for (uint32_t i = 0; i < shaderNum; i++) {
		shaders[i] = createShaderModule(createInfo.shaderStageCreateInfos[i]);
	}

	/*glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &result);
	if (!result) {
		int maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
		fprintf(stderr, "Invalid shader program: '%s'\n", infoLog.data());
		//return;
	}*/

	for (uint32_t i = 0; i < createInfo.attributesCount; i++) {
		glBindAttribLocation(program, createInfo.attributes[i].location, createInfo.attributes[i].name);
	}

	GLint result = 0;
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if (!result) {
		int maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
		fprintf(stderr, "Error linking shader program: '%s'\n", infoLog.data());
		return;
	}

	// Detach Shaders (Remove their references)
	for (size_t i = 0; i < shaderNum; i++) {
		glDetachShader(program, shaders[i]);
		glDeleteShader(shaders[i]);
	}

	delete[] shaders;

	for (size_t i = 0; i < createInfo.uniformBufferBindingCount; i++) {
		GLUniformBufferBinding *ubb = (GLUniformBufferBinding *)createInfo.uniformBufferBindings[i];
		GLuint index = glGetUniformBlockIndex(program, ubb->GetUniformName());
		glUniformBlockBinding(program, index, ubb->GetBindingLocation());
	}

	// TODO: Properly do this:
	glUseProgram(program);
	for (int i = 0; i < createInfo.textureBindingCount; i++) {
		GLTextureBindingLayout *texbinding = (GLTextureBindingLayout *)createInfo.textureBindings[i];
		for (int j = 0; j < texbinding->GetNumSubBindings(); j++) {
			TextureSubBinding sub = texbinding->GetSubBinding(j);
			int loc = glGetUniformLocation(program, sub.shaderLocation);
			glUniform1i(loc, sub.textureLocation);
		}
	}
}

void GLGraphicsPipeline::Bind() {
	glUseProgram(program);
	
	glViewport(0, 0, width, height);
	glScissor(scissorX, scissorY, scissorW, scissorH);
	switch (cullMode) {
	case CULL_NONE:
		glDisable(GL_CULL_FACE);
		break;
	case CULL_FRONT:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		break;
	case CULL_BACK:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		break;
	case CULL_BOTH:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT_AND_BACK);
		break;
	}
}

GLuint GLGraphicsPipeline::GetPrimitiveType() {
	return primitiveType;
}

GLGraphicsPipeline::~GLGraphicsPipeline() {
	glUseProgram(0);
	glDeleteProgram(program);
}