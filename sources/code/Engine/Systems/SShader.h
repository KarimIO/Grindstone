#ifndef _S_SHADER_H
#define _S_SHADER_H

#include "Core/GraphicsDLLPointer.h"
#include <map>
#include <vector>

enum PARAM_TYPE {
	PARAM_BOOL,
	PARAM_INT,
	PARAM_UINT,
	PARAM_FLOAT,
	PARAM_DOUBLE,
	PARAM_BVEC2,
	PARAM_BVEC3,
	PARAM_BVEC4,
	PARAM_IVEC2,
	PARAM_IVEC3,
	PARAM_IVEC4,
	PARAM_UVEC2,
	PARAM_UVEC3,
	PARAM_UVEC4,
	PARAM_VEC2,
	PARAM_VEC3,
	PARAM_VEC4,
	PARAM_DVEC2,
	PARAM_DVEC3,
	PARAM_DVEC4,
	PARAM_TEXTURE,
	PARAM_CUBEMAP
};

struct ParameterDescriptor {
	std::string description;
	PARAM_TYPE paramType;
	void *dataPtr;
};

struct ShaderFile {
	ShaderProgram *program;
	std::map<std::string, ParameterDescriptor> parameterDescriptorTable;
};

class ShaderManager {
	//std::vector<Shader *> shaders;
	std::vector<ShaderFile> shaderFiles;
public:
	void ParseShaderFile(const char *path);
};

#endif