#ifndef _GRAPHICS_DLL_POINTER_H
#define _GRAPHICS_DLL_POINTER_H

#include <OGLGraphicsWrapper.h>
#include <GLShader.h>

extern VertexArrayObject*	(*pfnCreateVAO)();
extern VertexBufferObject*	(*pfnCreateVBO)();
extern ShaderProgram*		(*pfnCreateShader)();

#endif