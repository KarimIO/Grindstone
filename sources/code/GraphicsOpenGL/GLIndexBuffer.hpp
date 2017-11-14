#ifndef _GL_INDEX_BUFFER_H
#define _GL_INDEX_BUFFER_H

#include "../GraphicsCommon/DLLDefs.h"
#include <stdint.h>
#include "../GraphicsCommon/IndexBuffer.h"

class GLIndexBuffer : public IndexBuffer {
	GLuint buffer;
public:
	GLIndexBuffer(IndexBufferCreateInfo createInfo);
	~GLIndexBuffer();

	void Bind();
};

#endif