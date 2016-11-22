#ifndef _VERTEX_BUFFER_OBJECT_H
#define _VERTEX_BUFFER_OBJECT_H

#include <vector>
#include <stdint.h>

enum : uint8_t {
	SIZE_BYTE = 0u,
	SIZE_UNSIGNED_BYTE,
	SIZE_SHORT,
	SIZE_UNSIGNED_SHORT,
	SIZE_INT,
	SIZE_UNSIGNED_INT,
	SIZE_FLOAT,
	SIZE_DOUBLE
};

struct VertexBufferObjectInitializer {
	void *data;
	uint64_t size;
	uint8_t strideSize;
	uint8_t dataSize;
};

class VertexBufferObject {
public:
	// Initialize
	virtual void Initialize(uint8_t size) = 0;

	// Pass VBO
	virtual void AddVBO(void *data, uint64_t size, uint8_t strideSize, uint8_t dataSize) = 0;
	virtual void AddVBO(VertexBufferObjectInitializer) = 0;
	virtual void AddVBO(std::vector<VertexBufferObjectInitializer>) = 0;

	// Bind VBO - returns next location
	virtual uint8_t Bind(uint8_t bindTo) = 0;

	// Unbind
	virtual void Unbind() = 0;

	// Cleanup
	virtual void Cleanup() = 0;
};

#endif