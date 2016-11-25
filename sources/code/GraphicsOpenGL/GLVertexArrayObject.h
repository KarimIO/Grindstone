#ifndef _GL_VERTEX_ARRAY_OBJECT_H
#define _GL_VERTEX_ARRAY_OBJECT_H

#include "../GraphicsCommon/GLDefDLL.h"
#include "../GraphicsCommon/VertexArrayObject.h"
#include "GLVertexBufferObject.h"

class GLVertexArrayObject : public VertexArrayObject {
public:
	unsigned int vao;

	uint8_t boundVBOs;
	std::vector <VertexBufferObject *> vbos;
public:
	// Initialize
	virtual void Initialize();

	// Pass VBO
	virtual void BindVBO(VertexBufferObject *vbo);

	// Bind VAO
	virtual void Bind();

	// Unbind
	virtual void Unbind();

	// Cleanup
	virtual void Cleanup();
	virtual void CleanupVBOs();
};

extern "C" GRAPHICS_EXPORT VertexArrayObject* createVAO();

#endif