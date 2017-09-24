#include "DxVertexArrayObject.h"
#include <iostream>

DxVertexArrayObject::DxVertexArrayObject(VertexArrayObjectCreateInfo createInfo) {
}

DxVertexArrayObject::~DxVertexArrayObject() {
}

void DxVertexArrayObject::Bind() {
	vertexBuffer->Bind();
	if (indexBuffer)
		indexBuffer->Bind();
}

void DxVertexArrayObject::BindResources(VertexArrayObjectCreateInfo createInfo) {
	vertexBuffer = (DxVertexBuffer *)createInfo.vertexBuffer;
	indexBuffer = (DxIndexBuffer *)createInfo.indexBuffer;
}

void DxVertexArrayObject::Unbind() {
}
