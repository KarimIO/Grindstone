#ifndef _UNIFORM_BUFFER_H
#define _UNIFORM_BUFFER_H

#include <stdint.h>
#include "../GraphicsCommon/GLDefDLL.h"

class UniformBuffer {
public:
	virtual void Initialize(int size) = 0;
	virtual unsigned int GetSize() = 0;
	virtual void Bind() = 0;
	virtual void Setdata(unsigned int offset, unsigned int size, void *value) = 0;
	virtual void Unbind() = 0;
};

extern "C" GRAPHICS_EXPORT UniformBuffer* createUniformBuffer();

#endif