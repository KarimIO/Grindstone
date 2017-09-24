#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"

struct VertexArrayObjectCreateInfo {
	VertexBuffer *vertexBuffer;
	IndexBuffer *indexBuffer;
};

class VertexArrayObject {
public:
	virtual ~VertexArrayObject() {};
	virtual void Bind() {};
	virtual void BindResources(VertexArrayObjectCreateInfo createInfo) {};
	virtual void Unbind() {};
};