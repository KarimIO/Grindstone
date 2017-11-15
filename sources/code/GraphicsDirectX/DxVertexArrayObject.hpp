#ifndef _GL_VERTEX_ARRAY_OBJECT_H
#define _GL_VERTEX_ARRAY_OBJECT_H

#include "../GraphicsCommon/VertexArrayObject.hpp"
#include "DxVertexBuffer.hpp"
#include "DxIndexBuffer.hpp"

class DxVertexArrayObject : public VertexArrayObject {
private:
public:
	DxVertexArrayObject(VertexArrayObjectCreateInfo createInfo);
	~DxVertexArrayObject();

	void Bind();
	void BindResources(VertexArrayObjectCreateInfo createInfo);
	void Unbind();

	DxVertexBuffer *vertexBuffer;
	DxIndexBuffer *indexBuffer;
};

#endif