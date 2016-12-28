#ifndef _GL_FRAMEBUFFER_H
#define _GL_FRAMEBUFFER_H

#include "../GraphicsCommon/Framebuffer.h"
#include "../GraphicsCommon/GLDefDLL.h"

class GLFramebuffer : public Framebuffer {
public:
	virtual void Initialize(unsigned short numBuffers);
	virtual void AddBuffer(unsigned int colorType, unsigned int colorFormat, unsigned int colorDataType, unsigned int width, unsigned int height);
	virtual void AddCubeBuffer(unsigned int colorType, unsigned int colorFormat, unsigned int colorDataType, unsigned int width, unsigned int height);
	virtual void AddDepthBuffer(unsigned int width, unsigned int height);
	virtual void Generate();
	virtual void BindTexture(unsigned int fboLoc);
	virtual void WriteBind();
	virtual void ReadBind();
	virtual void Unbind();
private:
	unsigned int fbo;
	unsigned int *textures;
	unsigned int renderBuffer;
	unsigned int numBuffers;
	unsigned int targetBuffer;
};

#endif