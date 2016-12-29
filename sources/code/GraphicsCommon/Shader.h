#ifndef _SHADER_H
#define _SHADER_H

#include <stdint.h>
#include <string>
#include "../GraphicsCommon/GLDefDLL.h"

enum ShaderType : uint8_t {
	SHADER_VERTEX = 0u,
	SHADER_FRAGMENT,
	SHADER_GEOMETRY
};

class ShaderProgram {
public:
	virtual bool AddShader(std::string path, std::string content, ShaderType) = 0;
	virtual bool Compile() = 0;
	virtual void Use() = 0;

	virtual void SetNumUniforms(int) = 0;
	virtual void CreateUniform(const char *) = 0;
	virtual void PassData(void *) = 0;
	virtual void SetUniform4m() = 0;
	virtual void SetInteger() = 0;
	virtual void SetVec3() = 0;

	virtual void Cleanup() = 0;
};

extern "C" GRAPHICS_EXPORT ShaderProgram* createShader();

#endif