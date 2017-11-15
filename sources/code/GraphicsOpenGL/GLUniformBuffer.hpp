#ifndef _GL_UNIFORM_BUFFER_H
#define _GL_UNIFORM_BUFFER_H

#include "../GraphicsCommon/UniformBuffer.hpp"

class GLUniformBufferBinding : public UniformBufferBinding {
	const char *uniformName;
	GLuint bindingLocation;
public:
	GLUniformBufferBinding(UniformBufferBindingCreateInfo);
	const char *GetUniformName();
	GLuint GetBindingLocation();
};

class GLUniformBuffer : public UniformBuffer {
private:
	GLuint ubo;
	GLuint bindingLocation;
	uint32_t size;
public:
	GLUniformBuffer(UniformBufferCreateInfo ci);
	void Bind();
	~GLUniformBuffer();

	void UpdateUniformBuffer(void * content);
};

#endif