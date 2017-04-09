#ifndef _GL_SHADER_H
#define _GL_SHADER_H

#include "../GraphicsCommon/Shader.h"
#include "../GraphicsCommon/UniformBuffer.h"

class GLShaderProgram : public ShaderProgram {
public:
	uint32_t program;
	uint32_t *shaders;
	uint8_t shaderNum;

	char *dataPtr;
	int *uniforms;
	uint8_t dataOffset;
	uint8_t uniformCounter;
public:
	virtual void Initialize(int numShaders);
	virtual bool AddShader(std::string *path, std::string *content, ShaderType);
	virtual bool Compile();
	virtual bool Validate();
	virtual void Use();

	virtual void BindAttribLocation(uint32_t, const char *);
	virtual void BindOutputLocation(uint32_t, const char *);

	virtual void SetNumUniforms(int);
	virtual void CreateUniform(const char *);
	virtual void PassData(void *);

	void PrepareBuffer(const char *name, UniformBuffer *buffer, unsigned int location);

	virtual void SetUniform4m();
	virtual void SetUniformFloat();
	virtual void SetInteger();
	virtual void SetVec4();
	virtual void SetVec3();
	virtual void SetVec2();
	virtual void SetFloatArray(unsigned int size);

	virtual void Cleanup();
};

#endif