#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include "../GraphicsCommon/GLDefDLL.h"

enum {
	// R
	COLOR_R8 = 0,
	COLOR_R8_S,
	COLOR_R8_I,
	COLOR_R8_UI,

	COLOR_R16,
	COLOR_R16_S,
	COLOR_R16_I,
	COLOR_R16_UI,
	COLOR_R16_F,

	COLOR_R32_I,
	COLOR_R32_UI,
	COLOR_R32_F,

	// RG
	COLOR_RG8,
	COLOR_RG8_S,
	COLOR_RG8_I,
	COLOR_RG8_UI,

	COLOR_RG16,
	COLOR_RG16_S,
	COLOR_RG16_I,
	COLOR_RG16_UI,
	COLOR_RG16_F,

	COLOR_RG32_I,
	COLOR_RG32_UI,
	COLOR_RG32_F,

	// RGB
	COLOR_RGB_3_3_2,
	COLOR_RGB4,
	COLOR_RGB5,
	COLOR_RGB10,
	COLOR_RGB512,

	COLOR_RGB8,
	COLOR_RGB8_S,
	COLOR_RGB8_I,
	COLOR_RGB8_UI,

	COLOR_RGB16,
	COLOR_RGB16_S,
	COLOR_RGB16_I,
	COLOR_RGB16_UI,
	COLOR_RGB16_F,

	COLOR_RGB32,
	COLOR_RGB32_S,
	COLOR_RGB32_I,
	COLOR_RGB32_UI,
	COLOR_RGB32_F,
};

class Framebuffer {
public:
	virtual void Initialize(unsigned short numBuffers) = 0;
	virtual void AddBuffer(unsigned int colorType, unsigned int colorFormat, unsigned int colorDataType, unsigned int width, unsigned int height) = 0;
	virtual void AddCubeBuffer(unsigned int colorType, unsigned int colorFormat, unsigned int colorDataType, unsigned int width, unsigned int height) = 0;
	virtual void AddDepthBuffer(unsigned int width, unsigned int height) = 0;
	virtual void AddDepthCubeBuffer(unsigned int width, unsigned int height) = 0;
	virtual void Generate() = 0;
	virtual void BindTexture(unsigned int fboLoc) = 0;
	virtual void BindTexture(unsigned int fboLoc, unsigned int bindLoc) = 0;
	virtual void BindDepth(unsigned int loc) = 0;
	virtual void BindDepthCube(unsigned int loc) = 0;
	virtual void WriteBind() = 0;
	virtual void WriteBindFace(unsigned int attachment, unsigned int face) = 0;
	virtual void ReadBind() = 0;
	virtual void Unbind() = 0;

	virtual void TestBlit(unsigned int x, unsigned int y, unsigned int srcWidth, unsigned int srcHeight, unsigned int width, unsigned int height, bool depth) = 0;
	virtual void SetAttachment(unsigned int a) = 0;
};

extern "C" GRAPHICS_EXPORT Framebuffer* createFramebuffer();

#endif