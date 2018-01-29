#ifndef _GL_FRAMEBUFFER_H
#define _GL_FRAMEBUFFER_H

#include "../GraphicsCommon/Framebuffer.hpp"
#include "../GraphicsCommon/DLLDefs.hpp"
#include "GLRenderTarget.hpp"
#include "GLDepthTarget.hpp"

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
	virtual void BindTextures(int i);
	virtual void Unbind();
private:
	GLuint fbo_;
	GLRenderTarget **render_target_lists_;
	uint32_t num_render_target_lists_;
	uint32_t num_total_render_targets;

	GLDepthTarget *depth_target_;
};

#endif