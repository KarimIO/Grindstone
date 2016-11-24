#ifndef _GL_VERTEX_BUFFER_OBJECT_H
#define _GL_VERTEX_BUFFER_OBJECT_H

#include <stdint.h>
#include "../GraphicsCommon/VertexBufferObject.h"
#include "gl3w.h"


class GLVertexBufferObject : public VertexBufferObject {
public:
	unsigned int *vboHandle;
	uint8_t size;
	unsigned int elementSize;
	dataSize dataSizeType;
public:
	// Initialize
	virtual void Initialize(uint8_t size);

	// Pass VBO
	virtual void AddVBO(void *data, uint64_t size, uint8_t strideSize, dataSize dataSize, drawType drawType);
	virtual void AddVBO(VertexBufferObjectInitializer);
	virtual void AddVBO(std::vector<VertexBufferObjectInitializer>);

	// Get Handle
	unsigned int *GetHandle();

	// Bind VBO
	virtual uint8_t Bind(uint8_t bindTo);
	virtual void Bind(uint8_t bindTo, uint8_t id, bool normalize, uint32_t stride, uint32_t offset);

	// Unbind
	virtual void Unbind();
	virtual void Unbind(uint8_t id);

	// Cleanup
	virtual void Cleanup();
};

#endif