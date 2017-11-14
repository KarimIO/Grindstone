#pragma once

#include "../GraphicsCommon/GraphicsPipeline.h"

class GLGraphicsPipeline : public GraphicsPipeline {
	GLuint program;
	GLuint primitiveType;

	float width, height;
	int32_t scissorX, scissorY;
	uint32_t scissorW, scissorH;
	CullMode cullMode;

	GLuint createShaderModule(ShaderStageCreateInfo shaderStageCreateInfo);
public:
	GLGraphicsPipeline(GraphicsPipelineCreateInfo createInfo);
	void Bind();
	GLuint GetPrimitiveType();
	~GLGraphicsPipeline();
};