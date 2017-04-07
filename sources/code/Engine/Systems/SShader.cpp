#include "SShader.h"

#include <fstream>

#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/error/en.h"

#include "glm/common.hpp"

#include "Core/Utilities.h"

enum SHADER_JSON_STATE {
	SHADER_JSON_MAIN = 0,
	SHADER_JSON_NAME,
	SHADER_JSON_SHADERS,
	SHADER_JSON_PROPERTIES,
	SHADER_JSON_PROPERTY,
	SHADER_JSON_PROPERTY_NAME,
	SHADER_JSON_PROPERTY_TYPE,
	SHADER_JSON_PROPERTY_DEFAULT,
	SHADER_JSON_VERTEX,
	SHADER_JSON_FRAGMENT,
	SHADER_JSON_GEOMETRY,
	SHADER_JSON_COMPUTE,
	SHADER_JSON_TESSEVAL,
	SHADER_JSON_TESSCTRL
};

#undef Bool
struct ShaderJSONHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, ShaderJSONHandler> {
private:
	unsigned char subIterator;
public:
	std::string vertexShader;
	std::string fragmentShader;
	std::string geometryShader;
	std::string computeShader;
	std::string eTessShader;
	std::string cTessShader;

	std::string szName;

	std::string paramName;
	std::string paramText;
	PARAM_TYPE paramType;
	void *paramDefault;

	SHADER_JSON_STATE state;

	glm::uvec4 uvec4;
	glm::ivec4 ivec4;
	glm::bvec4 bvec4;
	glm::vec4  vec4;
	glm::dvec4 dvec4;

	std::map<std::string, ShaderFile> *fileMap;
	ShaderFile *file;

	bool Null() { return true; }
	bool Bool(bool b) {
		if (state == SHADER_JSON_PROPERTY_DEFAULT) {
			if (paramType == PARAM_BOOL) {
				paramDefault = new bool(b);
			}
			else if (paramType == PARAM_BVEC2) {
				bvec4[subIterator++] = b;
				if (subIterator == 2)
					paramDefault = new glm::bvec2(bvec4.x, bvec4.y);
			}
			else if (paramType == PARAM_BVEC3) {
				bvec4[subIterator++] = b;
				if (subIterator == 3)
					paramDefault = new glm::bvec3(bvec4.x, bvec4.y, bvec4.z);
			}
			else if (paramType == PARAM_BVEC4) {
				bvec4[subIterator++] = b;
				if (subIterator == 4)
					paramDefault = new glm::bvec4(bvec4);
			}
		}
		return true;
	}
	bool Int(int i) {
		if (paramType == PARAM_INT) {
			paramDefault = new int(i);
		}
		else if (paramType == PARAM_IVEC2) {
			ivec4[subIterator++] = i;
			if (subIterator == 2)
				paramDefault = new glm::ivec2(ivec4.x, ivec4.y);
		}
		else if (paramType == PARAM_IVEC3) {
			ivec4[subIterator++] = i;
			if (subIterator == 3)
				paramDefault = new glm::ivec3(ivec4.x, ivec4.y, ivec4.z);
		}
		else if (paramType == PARAM_IVEC4) {
			ivec4[subIterator++] = i;
			if (subIterator == 4)
				paramDefault = new glm::ivec4(ivec4);
		}
		return true;
	}
	bool Uint(unsigned u) {
		if (paramType == PARAM_UINT) {
			paramDefault = new unsigned int(u);
		}
		else if (paramType == PARAM_UVEC2) {
			uvec4[subIterator++] = u;
			if (subIterator == 2)
				paramDefault = new glm::uvec2(uvec4.x, uvec4.y);
		}
		else if (paramType == PARAM_UVEC3) {
			uvec4[subIterator++] = u;
			if (subIterator == 3)
				paramDefault = new glm::uvec3(uvec4.x, uvec4.y, uvec4.z);
		}
		else if (paramType == PARAM_UVEC4) {
			uvec4[subIterator++] = u;
			if (subIterator == 4)
				paramDefault = new glm::uvec4(uvec4);
		}
		return true;
	}
	bool Int64(int64_t i) { Int((int)i); return true; }
	bool Uint64(uint64_t u) { Uint((int)u); return true; }
	bool Double(double d) {
		if (paramType == PARAM_FLOAT) {
			paramDefault = new float((float)d);
		}
		else if (paramType == PARAM_VEC2) {
			vec4[subIterator++] = (float)d;
			if (subIterator == 2)
				paramDefault = new glm::vec2(vec4.x, vec4.y);
		}
		else if (paramType == PARAM_VEC3) {
			vec4[subIterator++] = (float)d;
			if (subIterator == 3)
				paramDefault = new glm::vec3(vec4.x, vec4.y, vec4.z);
		}
		else if (paramType == PARAM_VEC4) {
			vec4[subIterator++] = (float)d;
			if (subIterator == 4)
				paramDefault = new glm::vec4(vec4);
		}
		else if (paramType == PARAM_DOUBLE) {
			paramDefault = new double(d);
		}
		else if (paramType == PARAM_DVEC2) {
			dvec4[subIterator++] = d;
			if (subIterator == 2)
				paramDefault = new glm::dvec2(dvec4.x, dvec4.y);
		}
		else if (paramType == PARAM_DVEC3) {
			dvec4[subIterator++] = d;
			if (subIterator == 3)
				paramDefault = new glm::dvec3(dvec4.x, dvec4.y, dvec4.z);
		}
		else if (paramType == PARAM_DVEC4) {
			dvec4[subIterator++] = d;
			if (subIterator == 4)
				paramDefault = new glm::dvec4(dvec4);
		}
		return true;
	}
	bool String(const char* str, rapidjson::SizeType length, bool copy) {
		if (state == SHADER_JSON_NAME) {
			szName = str;
			(*fileMap)[szName] = ShaderFile();
			file = &((*fileMap)[szName]);
		}
		else if (state == SHADER_JSON_PROPERTY_NAME) {
			paramName = str;
		}
		else if (state == SHADER_JSON_PROPERTY_TYPE) {
			if (std::string(str) == "bool")
				paramType = PARAM_BOOL;
			else if (std::string(str) == "int")
				paramType = PARAM_INT;
			else if (std::string(str) == "uint")
				paramType = PARAM_UINT;
			else if (std::string(str) == "float")
				paramType = PARAM_FLOAT;
			else if (std::string(str) == "double")
				paramType = PARAM_DOUBLE;
			else if (std::string(str) == "bvec2")
				paramType = PARAM_BVEC2;
			else if (std::string(str) == "bvec3")
				paramType = PARAM_BVEC3;
			else if (std::string(str) == "bvec4")
				paramType = PARAM_BVEC4;
			else if (std::string(str) == "ivec2")
				paramType = PARAM_IVEC2;
			else if (std::string(str) == "ivec3")
				paramType = PARAM_IVEC3;
			else if (std::string(str) == "ivec4")
				paramType = PARAM_IVEC4;
			else if (std::string(str) == "uvec2")
				paramType = PARAM_UVEC2;
			else if (std::string(str) == "uvec3")
				paramType = PARAM_UVEC3;
			else if (std::string(str) == "uvec4")
				paramType = PARAM_UVEC4;
			else if (std::string(str) == "vec2")
				paramType = PARAM_VEC2;
			else if (std::string(str) == "vec3")
				paramType = PARAM_VEC3;
			else if (std::string(str) == "vec4")
				paramType = PARAM_VEC4;
			else if (std::string(str) == "dvec2")
				paramType = PARAM_DVEC2;
			else if (std::string(str) == "dvec3")
				paramType = PARAM_DVEC3;
			else if (std::string(str) == "dvec4")
				paramType = PARAM_DVEC4;
			else if (std::string(str) == "texture")
				paramType = PARAM_TEXTURE;
			else if (std::string(str) == "cubemap")
				paramType = PARAM_CUBEMAP;
		}
		else if (state == SHADER_JSON_PROPERTY_DEFAULT) {
			if (paramType == PARAM_TEXTURE || paramType == PARAM_CUBEMAP)
				paramDefault = (void *)str;
		}
		else if (state == SHADER_JSON_VERTEX) {
			vertexShader = str;
		}
		else if (state == SHADER_JSON_FRAGMENT) {
			fragmentShader = str;
		}
		else if (state == SHADER_JSON_GEOMETRY) {
			geometryShader = str;
		}
		else if (state == SHADER_JSON_COMPUTE) {
			computeShader = str;
		}
		else if (state == SHADER_JSON_TESSEVAL) {
			eTessShader = str;
		}
		else if (state == SHADER_JSON_TESSCTRL) {
			cTessShader = str;
		}
		return true;
	}
	bool StartObject() {
		if (state == SHADER_JSON_PROPERTIES) state = SHADER_JSON_PROPERTY;
		return true;
	}
	bool Key(const char* str, rapidjson::SizeType length, bool copy) {
		if (state == SHADER_JSON_MAIN) {
			if (std::string(str) == "name") {
				state = SHADER_JSON_NAME;
			}
			else if (std::string(str) == "shaders") {
				state = SHADER_JSON_SHADERS;
			}
			else if (std::string(str) == "properties") {
				state = SHADER_JSON_PROPERTIES;
			}
		}
		else if (state == SHADER_JSON_SHADERS) {
			if (std::string(str) == "vertex") {
				state = SHADER_JSON_VERTEX;
			}
			else if (std::string(str) == "fragment") {
				state = SHADER_JSON_FRAGMENT;
			}
			else if (std::string(str) == "geometry") {
				state = SHADER_JSON_GEOMETRY;
			}
			else if (std::string(str) == "compute") {
				state = SHADER_JSON_COMPUTE;
			}
			else if (std::string(str) == "tesselationevaluate") {
				state = SHADER_JSON_TESSEVAL;
			}
			else if (std::string(str) == "tesselationcontrol") {
				state = SHADER_JSON_TESSCTRL;
			}
		}
		else if (state == SHADER_JSON_PROPERTIES) {
			paramName = str;
		}
		else if (state == SHADER_JSON_PROPERTY) {
			if (std::string(str) == "name") {
				state = SHADER_JSON_PROPERTY_NAME;
			}
			else if (std::string(str) == "type") {
				state = SHADER_JSON_PROPERTY_TYPE;
			}
			else if (std::string(str) == "default") {
				state = SHADER_JSON_PROPERTY_DEFAULT;
			}
		}
		return true;
	}
	bool EndObject(rapidjson::SizeType memberCount) {
		if (state == SHADER_JSON_PROPERTY) {
			state = SHADER_JSON_PROPERTIES;
			file->parameterDescriptorTable[paramName] = ParameterDescriptor(paramText, paramType, paramDefault);
		}
		return true;
	}
	bool StartArray() {
		subIterator = 0;
		return true;
	}
	bool EndArray(rapidjson::SizeType elementCount) {return true;}
};

ShaderProgram *ShaderManager::CreateShaderFromPaths(const char * name, const char * vsPath, const char * fsPath, const char * gsPath, const char * csPath, const char * tesPath, const char * tcsPath) {
	std::string vsContent, fsContent, gsContent, csContent, tesContent, tcsContent;
	int iterator = 2; // Must have at least a vertex and fragment shader.

	if (!ReadFileIncludable(vsPath, vsContent)) {
		fprintf(stderr, "Failed to read vertex shader %s of %s.\n", vsPath, name);
		return NULL;
	}

	if (!ReadFileIncludable(fsPath, fsContent)) {
		fprintf(stderr, "Failed to read fragment shader %s of %s.\n", fsPath, name);
		return NULL;
	}

	if (gsPath != "") {
		if (!ReadFileIncludable(gsPath, gsContent)) {
			fprintf(stderr, "Failed to read geometry shader %s of %s.\n", gsPath, name);
			return NULL;
		}
		iterator++;
	}

	if (csPath != "") {
		if (!ReadFileIncludable(csPath, csContent)) {
			fprintf(stderr, "Failed to read compute shader %s of %s.\n", csPath, name);
			return NULL;
		}
		iterator++;
	}

	if (tesPath != "") {
		if (!ReadFileIncludable(tesPath, tesContent)) {
			fprintf(stderr, "Failed to read tesselation evaluation shader %s of %s.\n", tesPath, name);
			return NULL;
		}
		iterator++;
	}

	if (tcsPath != "") {
		if (!ReadFileIncludable(tcsPath, tcsContent)) {
			fprintf(stderr, "Failed to read tesselation control shader %s of %s.\n", tcsPath, name);
			return NULL;
		}
		iterator++;
	}

	ShaderProgram *program = pfnCreateShader();
	program->Initialize(iterator);
	if (!program->AddShader(&std::string(vsPath), &vsContent, SHADER_VERTEX)) {
		fprintf(stderr, "Failed to add vertex shader %s of %s.\n", vsPath, name);
		return NULL;
	}
	if (!program->AddShader(&std::string(fsPath), &fsContent, SHADER_FRAGMENT)) {
		fprintf(stderr, "Failed to add fragment shader %s of %s.\n", fsPath, name);
		return NULL;
	}
	if (gsContent.size() > 0) {
		if (!program->AddShader(&std::string(gsPath), &gsContent, SHADER_GEOMETRY)) {
			fprintf(stderr, "Failed to add geometry shader %s of %s.\n", gsPath, name);
			return NULL;
		}
	}
	if (gsContent.size() > 0) {
		if (!program->AddShader(&std::string(csPath), &csContent, SHADER_COMPUTE)) {
			fprintf(stderr, "Failed to add control shader %s of %s.\n", csPath, name);
			return NULL;
		}
	}
	if (gsContent.size() > 0) {
		if (!program->AddShader(&std::string(tesPath), &tesContent, SHADER_TESS_EVALUATION)) {
			fprintf(stderr, "Failed to add tesselation evaluation shader %s of %s.\n", tesPath, name);
			return NULL;
		}
	}
	if (gsContent.size() > 0) {
		if (!program->AddShader(&std::string(tcsPath), &tcsContent, SHADER_TESS_CONTROL)) {
			fprintf(stderr, "Failed to add tesselation control shader %s of %s.\n", tcsPath, name);
			return NULL;
		}
	}
	if (!program->Compile()) {
		fprintf(stderr, "Failed to compile shader program: %s.\n", name);
		return NULL;
	}

	return program;
}

ShaderProgram *ShaderManager::ParseShaderFile(const char * path) {
	rapidjson::Reader reader;
	ShaderJSONHandler handler;
	handler.fileMap = &shaderFiles;
	std::ifstream input(path);
	rapidjson::IStreamWrapper isw(input);
	reader.Parse(isw, handler);
	ShaderProgram *program = CreateShaderFromPaths(handler.szName.c_str(), handler.vertexShader.c_str(), handler.fragmentShader.c_str(), handler.geometryShader.c_str(), handler.computeShader.c_str(), handler.eTessShader.c_str(), handler.cTessShader.c_str());
	handler.file->program = program;
	input.close();
	return program;
}

ParameterDescriptor::ParameterDescriptor() {}
ParameterDescriptor::ParameterDescriptor(std::string _desc, PARAM_TYPE _type, void * _ptr) {
	description = _desc;
	paramType = _type;
	dataPtr = _ptr;
}
