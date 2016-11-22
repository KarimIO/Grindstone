#ifndef _VERTEX_ARRAY_OBJECT_H
#define _VERTEX_ARRAY_OBJECT_H

#include <vector>
#include "VertexBufferObject.h"

class VertexArrayObject {
public:
	// Initialize
	virtual void Initialize() = 0;

	// Pass VBO
	virtual void BindVBO(VertexBufferObject *vbo) = 0;

	// Bind VAO
	virtual void Bind() = 0;

	// Unbind
	virtual void Unbind() = 0;

	// Cleanup
	virtual void Cleanup() = 0;
	virtual void CleanupVBOs() = 0;
};

#endif