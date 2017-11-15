#ifndef _GL_VERTEX_BUFFER_H
#define _GL_VERTEX_BUFFER_H

#include <stdint.h>
#include "../GraphicsCommon/VertexBuffer.hpp"


class GLVertexBuffer : public VertexBuffer {
private:
	GLuint buffer;
public:
	GLVertexBuffer(VertexBufferCreateInfo createInfo);
	~GLVertexBuffer();

	void Bind();
};

#endif