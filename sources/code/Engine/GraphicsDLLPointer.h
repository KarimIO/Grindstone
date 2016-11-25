#ifndef _GRAPHICS_DLL_POINTER_H
#define _GRAPHICS_DLL_POINTER_H

#include <OGLGraphicsWrapper.h>
#include <GLShader.h>

VertexArrayObject* (*pfnCreateVAO)();
VertexBufferObject* (*pfnCreateVBO)();
ShaderProgram* (*pfnCreateShader)();

#endif