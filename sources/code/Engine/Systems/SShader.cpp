#include "SShader.h"

#include <fstream>

#undef Bool

#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/error/en.h"

#include "glm/common.hpp"

#include "Core/Utilities.h"
#include "SGeometry.h"
#include "Core/Engine.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

/*enum SHADER_JSON_STATE {
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

/*ShaderProgram *MaterialManager::CreateShaderFromPaths(std::string name, std::string vsPath, std::string fsPath, std::string gsPath, std::string csPath, std::string tesPath, std::string tcsPath) {
	std::string vsContent, fsContent, gsContent, csContent, tesContent, tcsContent;
	int iterator = 2; // Must have at least a vertex and fragment shader.

	if (!ReadFileIncludable(vsPath, vsContent)) {
		fprintf(stderr, "Failed to read vertex shader %s of %s.\n", vsPath.c_str(), name.c_str());
		return NULL;
	}

	if (!ReadFileIncludable(fsPath, fsContent)) {
		fprintf(stderr, "Failed to read fragment shader %s of %s.\n", fsPath.c_str(), name.c_str());
		return NULL;
	}

	if (gsPath != "") {
		if (!ReadFileIncludable(gsPath, gsContent)) {
			fprintf(stderr, "Failed to read geometry shader %s of %s.\n", gsPath.c_str(), name.c_str());
			return NULL;
		}
		iterator++;
	}

	if (csPath != "") {
		if (!ReadFileIncludable(csPath, csContent)) {
			fprintf(stderr, "Failed to read compute shader %s of %s.\n", csPath.c_str(), name.c_str());
			return NULL;
		}
		iterator++;
	}

	if (tesPath != "") {
		if (!ReadFileIncludable(tesPath, tesContent)) {
			fprintf(stderr, "Failed to read tesselation evaluation shader %s of %s.\n", tesPath.c_str(), name.c_str());
			return NULL;
		}
		iterator++;
	}

	if (tcsPath != "") {
		if (!ReadFileIncludable(tcsPath, tcsContent)) {
			fprintf(stderr, "Failed to read tesselation control shader %s of %s.\n", tcsPath.c_str(), name.c_str());
			return NULL;
		}
		iterator++;
	}

	ShaderProgram *program = pfnCreateShader();
	program->Initialize(iterator);
	if (!program->AddShader(&vsPath, &vsContent, SHADER_VERTEX)) {
		fprintf(stderr, "Failed to add vertex shader %s of %s.\n", vsPath.c_str(), name.c_str());
		return NULL;
	}
	if (!program->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT)) {
		fprintf(stderr, "Failed to add fragment shader %s of %s.\n", fsPath.c_str(), name.c_str());
		return NULL;
	}
	if (gsContent.size() > 0) {
		if (!program->AddShader(&gsPath, &gsContent, SHADER_GEOMETRY)) {
			fprintf(stderr, "Failed to add geometry shader %s of %s.\n", gsPath.c_str(), name.c_str());
			return NULL;
		}
	}
	if (gsContent.size() > 0) {
		if (!program->AddShader(&csPath, &csContent, SHADER_COMPUTE)) {
			fprintf(stderr, "Failed to add control shader %s of %s.\n", csPath.c_str(), name.c_str());
			return NULL;
		}
	}
	if (gsContent.size() > 0) {
		if (!program->AddShader(&tesPath, &tesContent, SHADER_TESS_EVALUATION)) {
			fprintf(stderr, "Failed to add tesselation evaluation shader %s of %s.\n", tesPath.c_str(), name.c_str());
			return NULL;
		}
	}
	if (gsContent.size() > 0) {
		if (!program->AddShader(&tcsPath, &tcsContent, SHADER_TESS_CONTROL)) {
			fprintf(stderr, "Failed to add tesselation control shader %s of %s.\n", tcsPath.c_str(), name.c_str());
			return NULL;
		}
	}
	if (!program->Compile()) {
		fprintf(stderr, "Failed to compile shader program: %s.\n", name.c_str());
		return NULL;
	}

	return program;
}

ShaderProgram *MaterialManager::ParseShaderFile(std::string path) {
	rapidjson::Reader reader;
	ShaderJSONHandler handler;
	handler.fileMap = &shaderFiles;
	std::ifstream input(path);
	rapidjson::IStreamWrapper isw(input);
	reader.Parse(isw, handler);
	ShaderProgram *program = CreateShaderFromPaths(handler.szName, handler.vertexShader, handler.fragmentShader, handler.geometryShader, handler.computeShader, handler.eTessShader, handler.cTessShader);
	handler.file->program = program;
	input.close();
	return program;
}

ParameterDescriptor::ParameterDescriptor() {}
ParameterDescriptor::ParameterDescriptor(std::string _desc, PARAM_TYPE _type, void * _ptr) {
	description = _desc;
	paramType = _type;
	dataPtr = _ptr;
}*/

bool readFile(const std::string& filename, std::vector<char>& outputfile) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << filename.c_str() << "!\n";
		return false;
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	outputfile = buffer;

	return true;
}

RenderPassContainer *MaterialManager::Initialize(GraphicsWrapper *graphicsWrapper, VertexBindingDescription vbd, std::vector<VertexAttributeDescription> vads, UniformBufferBinding *ubb) {
	m_graphicsWrapper = graphicsWrapper;

	m_renderPasses.resize(1);

	m_renderPasses[0].pipelines.resize(1);
	m_renderPasses[0].pipelines[0].program = engine.pipeline;
	m_pipelineMap["../shaders/standard"].pipeline = 0;
	m_pipelineMap["../shaders/standard"].renderpass = 0;

	RenderPassCreateInfo rpci;
	std::vector<ClearColorValue> colorClearValues = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	ClearDepthStencil dscf;
	dscf.hasDepthStencilAttachment = true;
	dscf.depth = 1.0f;
	dscf.stencil = 0;
	rpci.m_colorClearValues = colorClearValues.data();
	rpci.m_colorClearCount = (uint32_t)colorClearValues.size();
	rpci.m_depthStencilClearValue = dscf;
	rpci.m_width = engine.settings.resolutionX;
	rpci.m_height = engine.settings.resolutionY;
	rpci.m_depthFormat = FORMAT_DEPTH_32;
	m_renderPasses[0].renderPass = graphicsWrapper->CreateRenderPass(rpci);

	DefaultFramebufferCreateInfo dfbci;
	dfbci.width = engine.settings.resolutionX;
	dfbci.height = engine.settings.resolutionY;
	dfbci.renderPass = m_renderPasses[0].renderPass;
	dfbci.depthFormat = FORMAT_DEPTH_32;

	Framebuffer **fboPtr;
	uint32_t fboCount;
	graphicsWrapper->CreateDefaultFramebuffers(dfbci, fboPtr, fboCount);

	m_renderPasses[0].framebuffers.resize(fboCount);
	size_t fboSize = fboCount * sizeof(Framebuffer *);
	memcpy(m_renderPasses[0].framebuffers.data(), fboPtr, fboSize);

	GraphicsPipelineCreateInfo gpci;
	gpci.scissorW = engine.settings.resolutionX;
	gpci.width = static_cast<float>(engine.settings.resolutionX);
	gpci.scissorH = engine.settings.resolutionY;
	gpci.height = static_cast<float>(engine.settings.resolutionY);
	gpci.renderPass = m_renderPasses[0].renderPass;
	gpci.attributes = vads.data();
	gpci.attributesCount = (uint32_t)vads.size();
	gpci.bindings = &vbd;
	gpci.bindingsCount = 1;

	ShaderStageCreateInfo vi;
	ShaderStageCreateInfo fi;
	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		vi.fileName = "../assets/shaders/vert.glsl";
		fi.fileName = "../assets/shaders/frag.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		vi.fileName = "../assets/shaders/vert.fxc";
		fi.fileName = "../assets/shaders/frag.fxc";
	}
	else {
		vi.fileName = "../assets/shaders/vert.spv";
		fi.fileName = "../assets/shaders/frag.spv";
	}
	std::vector<char> vfile;
	if (!readFile(vi.fileName, vfile))
		return nullptr;

	vi.content = vfile.data();
	vi.size = (uint32_t)vfile.size();
	vi.type = SHADER_VERTEX;

	std::vector<char> ffile;
	if (!readFile(fi.fileName, ffile))
		return nullptr;
		
	fi.content = ffile.data();
	fi.size = (uint32_t)ffile.size();
	fi.type = SHADER_FRAGMENT;

	std::vector<TextureSubBinding> bindings;
	bindings.reserve(4);
	bindings.emplace_back("tex0", 0);
	bindings.emplace_back("tex1", 1);
	bindings.emplace_back("tex2", 2);
	bindings.emplace_back("tex3", 3);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 2;
	tblci.bindings = bindings.data();
	tblci.bindingCount = (uint32_t)bindings.size();
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	TextureBindingLayout *tbl = m_graphicsWrapper->CreateTextureBindingLayout(tblci);

	std::vector<ShaderStageCreateInfo> stages = { vi, fi };
	gpci.shaderStageCreateInfos = stages.data();
	gpci.shaderStageCreateInfoCount = (uint32_t)stages.size();
	gpci.uniformBufferBindings = &ubb;
	gpci.uniformBufferBindingCount = 1;
	gpci.textureBindings = &tbl;
	gpci.textureBindingCount = 1;
	gpci.cullMode = CULL_BACK;
	gpci.primitiveType = PRIM_TRIANGLES;
	m_renderPasses[0].pipelines[0].program = graphicsWrapper->CreateGraphicsPipeline(gpci);
	
	PipelineContainer *pipeline = &m_renderPasses[0].pipelines[0];

	CommandBufferCreateInfo cbci;
	cbci.steps = nullptr;
	cbci.count = 0;
	cbci.numOutputs = fboCount;
	cbci.secondaryInfo.isSecondary = false;
	m_renderPasses[0].commandBuffer = m_graphicsWrapper->CreateCommandBuffer(cbci);

	cbci.steps = nullptr;
	cbci.count = 0;
	cbci.numOutputs = 1;
	cbci.secondaryInfo.isSecondary = true;
	cbci.secondaryInfo.fbos = m_renderPasses[0].framebuffers.data();
	cbci.secondaryInfo.fboCount = (uint32_t)m_renderPasses[0].framebuffers.size();
	cbci.secondaryInfo.renderPass = m_renderPasses[0].renderPass;
	pipeline->commandBuffer = m_graphicsWrapper->CreateCommandBuffer(cbci);

	return nullptr;
}

PipelineReference MaterialManager::CreatePipeline(std::string pipelineName) {
	if (m_pipelineMap.find(pipelineName) != m_pipelineMap.end())
		return m_pipelineMap[pipelineName];

	// Create It
	return PipelineReference();
}

MaterialReference MaterialManager::CreateMaterial(std::string path) {
	if (m_materialMap.find(path) != m_materialMap.end())
		return m_materialMap[path];

	std::ifstream input(path + ".gbm", std::ios::ate | std::ios::binary);

	if (!input.is_open()) {
		input.open(path + ".gjm", std::ios::ate | std::ios::binary);

		if (!input.is_open()) {
			std::cerr << "Failed to open material: " << path.c_str() << "!\n";
			return MaterialReference();
		}
	}

	std::cout << "Material reading from: " << path << "!\n";

	size_t fileSize = (size_t)input.tellg();
	std::vector<char> buffer(fileSize);

	input.seekg(0);
	input.read(buffer.data(), fileSize);

	// Extend shader info here
	char *words = buffer.data();
	std::string shader = words;
	words = strchr(words, '\0') + 1;
	std::string albedoPath = words;
	words = strchr(words, '\0') + 1;
	std::string normalPath = words;
	words = strchr(words, '\0') + 1;
	std::string roughnessPath = words;
	words = strchr(words, '\0') + 1;
	std::string specularPath = words;

	TextureBinding *textureBinding = nullptr;

	std::vector<SingleTextureBind> textures;
	textures.resize(4);
	std::string dir = path.substr(0, path.find_last_of("/") + 1);
	textures[0].texture = LoadTexture(dir + albedoPath);
	if (textures[0].texture != nullptr) {
		textures[1].texture = LoadTexture(dir + normalPath);
		textures[2].texture = LoadTexture(dir + roughnessPath);
		textures[3].texture = LoadTexture(dir + specularPath);

		textures[0].address = 0;
		textures[1].address = 1;
		textures[2].address = 2;
		textures[3].address = 3;

		TextureBindingCreateInfo ci;
		ci.textures = textures.data();
		ci.textureCount = (uint32_t)textures.size();
		textureBinding = m_graphicsWrapper->CreateTextureBinding(ci);
	}

	PipelineReference pipelineRef = CreatePipeline(shader);
	PipelineContainer *pipeline = GetPipeline(pipelineRef);

	MaterialReference ref;
	ref.pipelineReference = pipelineRef;
	ref.material = (uint32_t)pipeline->materials.size();

	pipeline->materials.emplace_back(textureBinding);

	m_materialMap[path] = ref;
	return ref;
}

RenderPassContainer *MaterialManager::GetRenderPass(uint8_t i) {
	return &m_renderPasses[i];
}

PipelineContainer * MaterialManager::GetPipeline(PipelineReference ref) {
	return &m_renderPasses[ref.renderpass].pipelines[ref.pipeline];
}

Material * MaterialManager::GetMaterial(MaterialReference ref) {
	return &m_renderPasses[ref.pipelineReference.renderpass].pipelines[ref.pipelineReference.pipeline].materials[ref.material];
}

void MaterialManager::RemoveRenderPass(uint8_t i) {
}

void MaterialManager::RemovePipeline(PipelineReference ref) {
}

void MaterialManager::RemoveMaterial(MaterialReference ref) {
}

void MaterialManager::RemoveMeshFromMaterial(MaterialReference ref, Mesh *mesh) {
	Material *mat = GetMaterial(ref);
	if (mat->m_meshes.size() > 1) {
		int index = 0; // Get Index from Reference by searching vector

		if (index + 1 != mat->m_meshes.size())
			std::swap(mat->m_meshes[index], mat->m_meshes.back());

		mat->m_meshes.pop_back();
	}
	else {
		// Remove entire Material
		RemoveMaterial(ref);
	}
}

struct DDSHeader {
	unsigned int width;
	unsigned int height;
	unsigned int linearSize;
	unsigned int mipMapCount;
	unsigned int fourCC;
};

#define FOURCC_DXT1 0x31545844
#define FOURCC_DXT3 0x33545844
#define FOURCC_DXT5 0x35545844

Texture * MaterialManager::LoadTexture(std::string path) {
	if (m_textureMap[path]) {
		printf("Texture reused: %s \n", path.c_str());
		return m_textureMap[path];
	}


	const char *filecode = path.c_str() + path.size() - 4;
	if (strncmp(filecode, "dds", 3) == 0) {
		// DDS
		FILE *fp;

		fp = fopen(path.c_str(), "rb");
		if (fp == NULL)
			return 0;

		// Verify file code in header
		char filecode[4];
		fread(filecode, 1, 4, fp);
		if (strncmp(filecode, "DDS ", 4) != 0) {
			fclose(fp);
			return 0;
		}

		DDSHeader header;

		fread(&header, 124, 1, fp);

		unsigned char * buffer;
		unsigned int bufsize;
		bufsize = header.mipMapCount > 1 ? header.linearSize * 2 : header.linearSize;
		buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
		fread(buffer, 1, bufsize, fp);

		fclose(fp);

		unsigned int components = (header.fourCC == FOURCC_DXT1) ? 3 : 4;
		ColorFormat format;
		switch (header.fourCC) {
		case FOURCC_DXT1:
			format = FORMAT_COLOR_RGBA_DXT1;
			break;
		case FOURCC_DXT3:
			format = FORMAT_COLOR_RGBA_DXT3;
			break;
		case FOURCC_DXT5:
			format = FORMAT_COLOR_RGBA_DXT5;
			break;
		default:
			free(buffer);
			return 0;
		}

		TextureCreateInfo createInfo;
		createInfo.data = buffer;
		createInfo.mipmaps = header.mipMapCount;
		createInfo.format = format;
		createInfo.width = header.width;
		createInfo.height = header.height;

		Texture *t = m_graphicsWrapper->CreateTexture(createInfo);
		m_textureMap[path] = t;

		stbi_image_free(buffer);
	}
	else {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		texChannels = 4;
		if (!pixels) {
			printf("Texture failed to load!: %s \n", path.c_str());
			return NULL;
		}

		printf("Texture loaded: %s \n", path.c_str());

		ColorFormat format;
		switch (texChannels) {
		case 1:
			format = FORMAT_COLOR_R8;
			break;
		case 2:
			format = FORMAT_COLOR_R8G8;
			break;
		case 3:
			format = FORMAT_COLOR_R8G8B8;
			break;
		default:
		case 4:
			format = FORMAT_COLOR_R8G8B8A8;
			break;
		}

		TextureCreateInfo createInfo;
		createInfo.format = format;
		createInfo.mipmaps = 0;
		createInfo.data = pixels;
		createInfo.width = texWidth;
		createInfo.height = texHeight;

		Texture *t = m_graphicsWrapper->CreateTexture(createInfo);
		m_textureMap[path] = t;

		stbi_image_free(pixels);

		return t;
	}
	return nullptr;
}

void MaterialManager::DrawImmediate() {
	for (auto const &renderPass : m_renderPasses) {
		//renderPass->Bind();
		for (auto const &pipeline : renderPass.pipelines) {
			pipeline.program->Bind();
			for (auto const &material : pipeline.materials) {
				if (material.m_textureBinding != nullptr)
					m_graphicsWrapper->BindTextureBinding(material.m_textureBinding);
				for (auto const &mesh : material.m_meshes) {
					mesh->Draw();
				}
			}
		}
	}
}

void MaterialManager::DrawDeferred() {
	uint32_t currentFrame = m_graphicsWrapper->GetImageIndex();

	for (auto const &renderPass : m_renderPasses) {
		std::vector<CommandBuffer *> cmds;
		cmds.reserve(renderPass.pipelines.size());
		for (auto const &pipeline : renderPass.pipelines) {
			pipeline.commandBuffer->SetCommandBuffer(currentFrame);
			pipeline.commandBuffer->Reset();
			pipeline.commandBuffer->Begin();
			pipeline.commandBuffer->BindGraphicsPipeline(pipeline.program);
			for (auto const &material : pipeline.materials) {
				//pipeline.commandBuffer->BindDescriptorSet(material.m_textureBinding);
				for (auto const &mesh : material.m_meshes) {
					mesh->DrawDeferred(pipeline.commandBuffer);
				}
			}
			cmds.push_back(pipeline.commandBuffer);
		}
		renderPass.commandBuffer->SetCommandBuffer(currentFrame);
		renderPass.commandBuffer->Reset();
		renderPass.commandBuffer->Begin();
		Framebuffer **f = (Framebuffer **)renderPass.framebuffers.data();
		renderPass.commandBuffer->BindRenderPass(renderPass.renderPass, f, (uint32_t)renderPass.framebuffers.size());
		renderPass.commandBuffer->BindCommandBuffers(cmds.data(), (uint32_t)cmds.size());
		renderPass.commandBuffer->UnbindRenderPass();
		renderPass.commandBuffer->End();
	}
}

void MaterialManager::Shutdown() {
	for (const auto &texture : m_textureMap) {
		m_graphicsWrapper->DeleteTexture(texture.second);
	}
}

Material::Material(TextureBinding * textureBinding) {
	m_meshes.reserve(1);
	m_textureBinding = textureBinding;
}