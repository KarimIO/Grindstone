#ifndef _GL_SHADER_H
#define _GL_SHADER_H

#include "../GraphicsCommon/GLDefDLL.h"
#include "../GraphicsCommon/Shader.h"
#include "gl3w.h"

class GLShaderProgram : public ShaderProgram {
public:
	uint32_t program;
	uint32_t shaders[2];
	uint8_t shaderNum;
public:
	GLShaderProgram();
	virtual bool AddShader(std::string path, std::string content, ShaderType);
	virtual bool Compile();
	virtual void Use();

	virtual void Cleanup();
};

extern "C" GRAPHICS_EXPORT ShaderProgram* createShader();

#endif