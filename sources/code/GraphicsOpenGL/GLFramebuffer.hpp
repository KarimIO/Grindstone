#ifndef _GL_FRAMEBUFFER_H
#define _GL_FRAMEBUFFER_H

#include "../GraphicsCommon/Framebuffer.hpp"
#include "../GraphicsCommon/DLLDefs.hpp"

class GLFramebuffer : public Framebuffer {
	GLuint m_fbo;
	GLuint *m_textures;
	GLuint m_numTextures;
	GLuint m_depthTexture;

	uint32_t m_width, m_height;
public:
	GLFramebuffer(FramebufferCreateInfo);
	~GLFramebuffer();
	void Clear();
	void Blit(int i, int x, int y, int w, int h);
	void BindWrite();
	void BindRead();
	void BindTextures();
	void Unbind();
};

#endif