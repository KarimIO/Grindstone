#ifndef _SHADER_H
#define _SHADER_H

#include <stdint.h>
#include <string>
#include "../GraphicsCommon/GLDefDLL.h"

enum ShaderType : uint8_t {
	SHADER_VERTEX = 0u,
	SHADER_FRAGMENT,
	SHADER_TESS_CONTROL,
	SHADER_TESS_EVALUATION,
	SHADER_GEOMETRY
};

class ShaderProgram {
public:
	virtual void Initialize(int numShaders) = 0;
	virtual bool AddShader(std::string *path, std::string *content, ShaderType) = 0;
	virtual bool Compile() = 0;
	virtual bool Validate() = 0;
	virtual void Use() = 0;

	virtual void BindAttribLocation(uint32_t, const char *) = 0;
	virtual void BindOutputLocation(uint32_t, const char *) = 0;

	virtual void SetNumUniforms(int) = 0;
	virtual void CreateUniform(const char *) = 0;
	virtual void PassData(void *) = 0;
	virtual void SetUniform4m() = 0;
	virtual void SetUniformFloat() = 0;
	virtual void SetInteger() = 0;
	virtual void SetVec4() = 0;
	virtual void SetVec3() = 0;
	virtual void SetVec2() = 0;

	virtual void Cleanup() = 0;
};

extern "C" GRAPHICS_EXPORT ShaderProgram* createShader();

#endif