#pragma once

#include "RenderPass.h"
#include "VertexBuffer.h"
#include "UniformBuffer.h"
#include <string>
#include <vector>
#include <iostream>
#include "Texture.h"
#include "UniformBuffer.h"

enum ShaderStageType {
	SHADER_VERTEX = 0,
	SHADER_TESS_EVALUATION,
	SHADER_TESS_CONTROL,
	SHADER_GEOMETRY,
	SHADER_FRAGMENT
};

struct ShaderStageCreateInfo {
	const char *fileName;
	const char *content;
	uint32_t size;
	ShaderStageType type;
};

enum PrimitiveType {
	PRIM_TRIANGLES = 0,
	PRIM_TRIANGLE_STRIPS,
	PRIM_PATCHES
};

enum CullMode {
	CULL_NONE = 0,
	CULL_FRONT,
	CULL_BACK,
	CULL_BOTH
};

struct GraphicsPipelineCreateInfo {
	PrimitiveType primitiveType;
	CullMode cullMode;
	RenderPass *renderPass;
	float width, height;
	int32_t scissorX = 0, scissorY = 0;
	uint32_t scissorW, scissorH;
	ShaderStageCreateInfo *shaderStageCreateInfos;
	uint32_t shaderStageCreateInfoCount;

	UniformBufferBinding **uniformBufferBindings;
	uint32_t uniformBufferBindingCount;
	
	TextureBindingLayout **textureBindings;
	uint32_t textureBindingCount;

	VertexBindingDescription *bindings;
	uint32_t bindingsCount;
	VertexAttributeDescription *attributes;
	uint32_t attributesCount;
};

class GraphicsPipeline {
public:
	virtual void Bind() {};
	virtual ~GraphicsPipeline() {}
};