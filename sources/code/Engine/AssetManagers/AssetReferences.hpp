#ifndef _ASSET_REFERENCES_H
#define _ASSET_REFERENCES_H

#include <stdint.h>
#include <VertexBuffer.hpp>
#include <UniformBuffer.hpp>

enum ProgramType {
	TYPE_UNLIT = 0,
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
	VertexBindingDescription *vbds;
	unsigned int vbds_count;
	VertexAttributeDescription *vads;
	unsigned int vads_count;
	UniformBufferBinding **ubbs;
	unsigned int ubb_count;
};

typedef unsigned int ComponentHandle;
typedef unsigned int GameObjectHandle;

#endif