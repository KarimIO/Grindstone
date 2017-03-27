#ifndef _VERTEX_BUFFER_OBJECT_H
#define _VERTEX_BUFFER_OBJECT_H

#include <vector>
#include <stdint.h>
#include "GLDefDLL.h"

enum dataSize : uint8_t {
	SIZE_BYTE = 0u,
	SIZE_UNSIGNED_BYTE,
	SIZE_SHORT,
	SIZE_UNSIGNED_SHORT,
	SIZE_INT,
	SIZE_UNSIGNED_INT,
	SIZE_FLOAT,
	SIZE_DOUBLE
};

enum drawType : uint8_t {
	DRAW_STATIC = 0u,
	DRAW_DYNAMIC,
	DRAW_STREAM
};

struct VertexBufferObjectInitializer {
	bool isIndicesVBO;
	void *data;
	uint64_t size;
	uint8_t strideSize;
	dataSize dataSz;
	drawType drawTp;
};

class VertexBufferObject {
public:
	// Initialize
	virtual void Initialize(uint8_t size) = 0;

	// Pass VBO
	virtual void AddVBO(void *data, uint64_t size, uint8_t strideSize, dataSize, drawType) = 0;
	virtual void AddIBO(void *data, uint64_t size, drawType) = 0;
	virtual void AddVBO(VertexBufferObjectInitializer) = 0;
	virtual void AddVBO(std::vector<VertexBufferObjectInitializer>) = 0;

	// Bind VBO - returns next location
	virtual uint8_t Bind(uint8_t bindTo) = 0;
	virtual void Bind(uint8_t bindTo, uint8_t id, bool normalize, uint32_t stride, uint32_t offset) = 0;
	virtual void IBind(uint8_t bindTo, uint8_t id, uint32_t stride, uint32_t offset) = 0;

	// Unbind
	virtual void Unbind() = 0;

	// Cleanup
	virtual void Cleanup() = 0;
};

extern "C" GRAPHICS_EXPORT VertexBufferObject* createVBO();

#endif