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
	virtual void AddDepthCubeBuffer(unsigned int width, unsigned int height);
	virtual void Generate();
	virtual void BindTexture(unsigned int fboLoc);
	virtual void BindDepth(unsigned int loc);
	virtual void BindDepthCube(unsigned int loc);
	virtual void BindTexture(unsigned int fboLoc, unsigned int bindLoc);
	virtual void WriteBind();
	virtual void WriteBindFace(unsigned int attachment, unsigned int face);
	virtual void ReadBind();
	virtual void Unbind();
	virtual void TestBlit(int width, int height);
private:
	unsigned int fbo;
	unsigned int *textures;
	unsigned int depthBuffer;
	unsigned int numBuffers;
	unsigned int targetBuffer;
};

#endif