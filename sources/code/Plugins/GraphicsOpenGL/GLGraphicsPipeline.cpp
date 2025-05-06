#include <vector>
#include <iostream>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <cstring>
#include <GL/gl3w.h>

#include "GLBuffer.hpp"
#include "GLTexture.hpp"
#include "GLGraphicsPipeline.hpp"
#include "GLCore.hpp"
#include "GLFormats.hpp"

using namespace Grindstone::GraphicsAPI;

OpenGL::GraphicsPipeline::GraphicsPipeline(const GraphicsPipeline::CreateInfo& createInfo) {
	CreatePipeline(createInfo);
}

void OpenGL::GraphicsPipeline::CreatePipeline(const GraphicsPipeline::CreateInfo& createInfo) {
	const VertexInputLayout& vertexInputLayout = createInfo.vertexInputLayout;
	const PipelineData& pipelineData = createInfo.pipelineData;

	width = static_cast<GLsizei>(pipelineData.width);
	height = static_cast<GLsizei>(pipelineData.height);
	scissorWidth = static_cast<GLsizei>(pipelineData.scissorW);
	scissorHeight = static_cast<GLsizei>(pipelineData.scissorH);
	scissorX = static_cast<GLint>(pipelineData.scissorX);
	scissorY = static_cast<GLint>(pipelineData.scissorY);

	depthCompareOp = TranslateCompareOpToOpenGL(pipelineData.depthCompareOp);
	isDepthTestEnabled = pipelineData.isDepthTestEnabled;
	isDepthWriteEnabled = pipelineData.isDepthWriteEnabled;
	isStencilEnabled = pipelineData.isStencilEnabled;
	isDepthBiasEnabled = pipelineData.isDepthBiasEnabled;
	isDepthClampEnabled = pipelineData.isDepthClampEnabled;

	depthBiasConstantFactor = pipelineData.depthBiasConstantFactor;
	depthBiasSlopeFactor = pipelineData.depthBiasSlopeFactor;
	depthBiasClamp = pipelineData.depthBiasClamp;

	primitiveType = TranslateGeometryTypeToOpenGL(pipelineData.primitiveType);
	polygonFillMode = TranslatePolygonModeToOpenGL(pipelineData.polygonFillMode);
	cullMode = TranslateCullModeToOpenGL(pipelineData.cullMode);

	// TODO: Support colorMask for different attachments.
	// colorMaskRed = static_cast<GLboolean>(pipelineData.colorMask & ColorMask::Red);
	// colorMaskBlue = static_cast<GLboolean>(pipelineData.colorMask & ColorMask::Blue);
	// colorMaskGreen = static_cast<GLboolean>(pipelineData.colorMask & ColorMask::Green);
	// colorMaskAlpha = static_cast<GLboolean>(pipelineData.colorMask & ColorMask::Alpha);
	// 
	// TODO: Support blends for different attachments.
	// blendColorOp = TranslateBlendOpToOpenGL(pipelineData.blendData.colorOperation);
	// blendColorSrc = TranslateBlendFactorToOpenGL(pipelineData.blendData.colorFactorSrc);
	// blendColorDst = TranslateBlendFactorToOpenGL(pipelineData.blendData.colorFactorDst);
	// blendAlphaOp = TranslateBlendOpToOpenGL(pipelineData.blendData.alphaOperation);
	// blendAlphaSrc = TranslateBlendFactorToOpenGL(pipelineData.blendData.alphaFactorSrc);
	// blendAlphaDst = TranslateBlendFactorToOpenGL(pipelineData.blendData.alphaFactorDst);

	program = glCreateProgram();
	glObjectLabel(GL_PROGRAM, program, -1, pipelineData.debugName);

	uint32_t shaderNum = pipelineData.shaderStageCreateInfoCount;
	GLuint* shaders = new GLuint[shaderNum];
	for (uint32_t i = 0; i < shaderNum; i++) {
		shaders[i] = CreateShaderModule(pipelineData.shaderStageCreateInfos[i]);
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

	for (size_t i = 0; i < shaderNum; i++) {
		glDeleteShader(shaders[i]);
	}

	delete[] shaders;
}

GLuint OpenGL::GraphicsPipeline::CreateShaderModule(const GraphicsPipeline::ShaderStageData& pipelineData) {
	int shaderType;
	switch (pipelineData.type) {
	default:
	case ShaderStage::Vertex:
		shaderType = GL_VERTEX_SHADER;
		break;
	case ShaderStage::Fragment:
		shaderType = GL_FRAGMENT_SHADER;
		break;
	case ShaderStage::TesselationEvaluation:
		shaderType = GL_TESS_EVALUATION_SHADER;
		break;
	case ShaderStage::TesselationControl:
		shaderType = GL_TESS_CONTROL_SHADER;
		break;
	case ShaderStage::Geometry:
		shaderType = GL_GEOMETRY_SHADER;
		break;
	}

	GLuint shader = glCreateShader(shaderType);
	if (pipelineData.fileName != nullptr) {
		glObjectLabel(GL_SHADER, shader, -1, pipelineData.fileName);
	}

	bool shouldUseTextShaders = false;

	if (shouldUseTextShaders) {
		const GLint size = pipelineData.size;
		glShaderSource(shader, 1, &pipelineData.content, &size);
		glCompileShader(shader);

		GLint result = GL_FALSE;
		int infoLength;

		// Check Shader
		glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);
		if (!result) {
			printf("Error Report in Shader %s\n", pipelineData.fileName);
			std::vector<char> VertexShaderErrorMessage(infoLength + 1);
			glGetShaderInfoLog(shader, infoLength, NULL, VertexShaderErrorMessage.data());
			printf("%s\n", VertexShaderErrorMessage.data());
			return false;
		}
	}
	else {
		glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, pipelineData.content, pipelineData.size);
		glSpecializeShader(shader, "main", 0, 0, 0);
	}

	glAttachShader(program, shader);

	return shader;
}

void OpenGL::GraphicsPipeline::Bind() {
	glUseProgram(program);

	glViewport(0, 0, scissorX, scissorY);
	glScissor(scissorX, scissorY, scissorWidth, scissorHeight);

	if (cullMode == GL_NONE) {
		glDisable(GL_CULL_FACE);
	}
	else {
		glEnable(GL_CULL_FACE);
		glCullFace(cullMode);
	}

	if (blendColorOp == GL_NONE || blendAlphaOp == GL_NONE) {
		glDisable(GL_BLEND);
	}
	else {
		glEnable(GL_BLEND);
		glBlendEquationSeparate(blendColorOp, blendAlphaOp);
		glBlendFuncSeparate(blendColorSrc, blendColorDst, blendAlphaSrc, blendAlphaDst);
	}

	if (isDepthTestEnabled) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	if (isStencilEnabled) {
		glEnable(GL_STENCIL_TEST);
	}
	else {
		glDisable(GL_STENCIL_TEST);
	}

	glDepthMask(isDepthWriteEnabled);
	glDepthFunc(depthCompareOp);
	glPolygonMode(GL_FRONT_AND_BACK, polygonFillMode);

	glColorMask(colorMaskRed, colorMaskGreen, colorMaskBlue, colorMaskAlpha);
	if (isDepthClampEnabled) {
		glEnable(GL_DEPTH_CLAMP);
	}
	else {
		glDisable(GL_DEPTH_CLAMP);
	}

	if (isDepthBiasEnabled) {
		glEnable(GL_POLYGON_OFFSET_FILL);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glEnable(GL_POLYGON_OFFSET_POINT);
		glPolygonOffset(depthBiasSlopeFactor, depthBiasConstantFactor);
	}
	else {
		glDisable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glDisable(GL_POLYGON_OFFSET_POINT);
	}
}

GLuint OpenGL::GraphicsPipeline::GetPrimitiveType() {
	return primitiveType;
}

OpenGL::GraphicsPipeline::~GraphicsPipeline() {
	glUseProgram(0);
	glDeleteProgram(program);
}
