#ifndef _SHADER_H
#define _SHADER_H

#include <stdint.h>
#include <string>

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

	virtual void Cleanup() = 0;
};

#endif