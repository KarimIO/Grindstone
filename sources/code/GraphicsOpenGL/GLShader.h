#ifndef _GL_SHADER_H
#define _GL_SHADER_H

#include "../GraphicsCommon/Shader.h"

class GLShaderProgram : public ShaderProgram {
public:
	uint32_t program;
	uint32_t shaders[2];
	uint8_t shaderNum;

	char *dataPtr;
	int *uniforms;
	uint8_t dataOffset;
	uint8_t uniformCounter;
public:
	GLShaderProgram();
	virtual bool AddShader(std::string path, std::string content, ShaderType);
	virtual bool Compile();
	virtual void Use();

	virtual void SetNumUniforms(int);
	virtual void CreateUniform(const char *);
	virtual void PassData(void *);
	virtual void SetUniform4m();
	virtual void SetInteger();

	virtual void Cleanup();
};

#endif