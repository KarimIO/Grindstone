#ifndef _GL_UNIFORM_BUFFER_H
#define _GL_UNIFORM_BUFFER_H

#include "../GraphicsCommon/UniformBuffer.h"

class GLUniformBuffer : public UniformBuffer {
public:
	virtual void Initialize(int size);
	virtual unsigned int GetSize();
	virtual void Bind();
	virtual void Setdata(unsigned int offset, unsigned int size, void *value);
	virtual void Unbind();

// DATA
	GLuint block;
	unsigned int size;
};

#endif