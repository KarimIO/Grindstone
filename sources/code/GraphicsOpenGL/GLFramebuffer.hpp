#ifndef _GL_FRAMEBUFFER_H
#define _GL_FRAMEBUFFER_H

#include "../GraphicsCommon/Framebuffer.hpp"
#include "../GraphicsCommon/DLLDefs.hpp"

class GLFramebuffer : public Framebuffer {
public:
	GLFramebuffer(FramebufferCreateInfo);
	~GLFramebuffer();
	virtual void Clear();
	virtual void CopyFrom(Framebuffer *);
	virtual void Blit(uint32_t i, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
	virtual void Bind();
	virtual void BindWrite();
	virtual void BindRead();
	virtual void Unbind();
private:
	GLuint fbo_;
};

#endif