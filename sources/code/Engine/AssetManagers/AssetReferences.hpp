#ifndef _ASSET_REFERENCES_H
#define _ASSET_REFERENCES_H

#include <stdint.h>
#include <GraphicsCommon/VertexBuffer.hpp>
#include <GraphicsCommon/UniformBuffer.hpp>

enum ProgramType {
	TYPE_MISSING = 0,
	TYPE_UNLIT,
	TYPE_OPAQUE,
	TYPE_TRANSPARENT,
	TYPE_MISC
};

struct PipelineReference {
	uint8_t renderpass = 0;
	uint8_t pipeline = 0;
	ProgramType pipeline_type;
};

struct MaterialReference {
	PipelineReference pipelineReference;
	uint16_t material = 0;
};

typedef uint32_t ModelReference;

struct GeometryInfo {
	Grindstone::GraphicsAPI::VertexBufferLayout *vertex_layout;
	unsigned int vertex_layout_count;
	Grindstone::GraphicsAPI::UniformBufferBinding **ubbs;
	unsigned int ubb_count;
};

typedef unsigned int ComponentHandle;
typedef unsigned int GameObjectHandle;

#endif