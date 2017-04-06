#include "SShader.h"

#include <fstream>

#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/error/en.h"

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

	bool Null() { return true; }
	bool Bool(bool b) {
		return true;
	}
	bool Int(int i) {
		return true;
	}
	bool Uint(unsigned u) { Int((int)u); return true; }
	bool Int64(int64_t i) { Int((int)i); return true; }
	bool Uint64(uint64_t u) { Int((int)u); return true; }
	bool Double(double d) {
		return true;
	}
	bool String(const char* str, rapidjson::SizeType length, bool copy) {
		if (state == SHADER_JSON_NAME) {
			szName = str;
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
		if (state == SHADER_JSON_PROPERTY) state = SHADER_JSON_PROPERTIES;
		return true;
	}
	bool StartArray() {
		subIterator = 0;
		return true;
	}
	bool EndArray(rapidjson::SizeType elementCount) {return true;}
};

void ShaderManager::ParseShaderFile(const char * path) {
	rapidjson::Reader reader;
	ShaderJSONHandler handler;
	std::ifstream input(path);
	rapidjson::IStreamWrapper isw(input);
	reader.Parse(isw, handler);
	input.close();
}
