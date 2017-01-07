#ifndef _GRAPHICS_DLL_POINTER_H
#define _GRAPHICS_DLL_POINTER_H

#include <GraphicsWrapper.h>
#include <VertexArrayObject.h>
#include <VertexBufferObject.h>
#include <GLShader.h>
#include <GLTexture.h>
#include <GLFramebuffer.h>

extern VertexArrayObject*	(*pfnCreateVAO)();
extern VertexBufferObject*	(*pfnCreateVBO)();
extern ShaderProgram*		(*pfnCreateShader)();
extern Texture*				(*pfnCreateTexture)();
extern Framebuffer*			(*pfnCreateFramebuffer)();

#endif