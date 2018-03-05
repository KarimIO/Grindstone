#include "SMaterial.hpp"

#include <fstream>

#undef Bool

#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/error/en.h"

#include "glm/common.hpp"

#include "Core/Utilities.hpp"
#include "SGeometry.hpp"
#include "Core/Engine.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>
#include <fstream>

#include "Core/Engine.hpp"

enum SHADER_JSON_STATE {
	SHADER_JSON_MAIN = 0,
	SHADER_JSON_NAME,
	SHADER_JSON_TYPE,
	SHADER_JSON_SHADERS,
	// Draw Modes
	SHADER_JSON_SHADER_DEFERRED,
	SHADER_JSON_SHADER_FORWARD,
	SHADER_JSON_SHADER_SHADOW,
	// Platforms
	SHADER_JSON_SHADER_OPENGL,
	SHADER_JSON_SHADER_DIRECTX,
	SHADER_JSON_SHADER_VULKAN,
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

enum StateJsonRenderType {
	STATE_JSON_RENDER_FORWARD = 0,
	STATE_JSON_RENDER_DEFERRED
};

struct ShaderJSONHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, ShaderJSONHandler> {
private:
	unsigned char subIterator;
public:
	std::string path;
	std::string dir;
	StateJsonRenderType staterendertype;

	std::string paramName;
	std::string paramText;
	PARAM_TYPE paramType;
	void *paramDefault;

	SHADER_JSON_STATE state = SHADER_JSON_MAIN;
	SHADER_JSON_STATE shader_type;

	glm::uvec4 uvec4;
	glm::ivec4 ivec4;
	glm::bvec4 bvec4;
	glm::vec4  vec4;
	glm::dvec4 dvec4;

	bool in_textures;
	unsigned int texture_id = 0;

	bool misc;

	RenderPassContainer *render_pass;
	PipelineContainer *pipeline;

	bool Null() { return true; }
	bool Bool(bool b) {
		if (state == SHADER_JSON_PROPERTY_DEFAULT) {
			state = SHADER_JSON_PROPERTY;
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
		state = SHADER_JSON_PROPERTY;
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
		state = SHADER_JSON_PROPERTY;
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
		state = SHADER_JSON_PROPERTY;
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
			if (subIterator == 4) {
				paramDefault = new glm::dvec4(dvec4);
			}
		}
		state = SHADER_JSON_PROPERTY;
		return true;
	}
	bool String(const char* str, rapidjson::SizeType length, bool copy) {
		if (state == SHADER_JSON_NAME) {
			pipeline->name_text = str;
			state = SHADER_JSON_MAIN;
		}
		else if (state == SHADER_JSON_TYPE) {
			std::string type = str;
			if (!misc) {
				if (type == "unlit") {
					render_pass->pipelines_unlit.push_back(PipelineContainer());
					pipeline = &render_pass->pipelines_unlit.back();
					pipeline->type = TYPE_UNLIT;
					pipeline->reference.pipeline_type = TYPE_UNLIT;
					pipeline->reference.renderpass = 0;
					pipeline->reference.pipeline = render_pass->pipelines_unlit.size() - 1;
				}
				else if (type == "opaque") {
					render_pass->pipelines_deferred.push_back(PipelineContainer());
					pipeline = &render_pass->pipelines_deferred.back();
					pipeline->type = TYPE_OPAQUE;
					pipeline->reference.pipeline_type = TYPE_OPAQUE;
					pipeline->reference.renderpass = 0;
					pipeline->reference.pipeline = render_pass->pipelines_deferred.size() - 1;
				}
				else if (type == "transparent") {
					render_pass->pipelines_forward.push_back(PipelineContainer());
					pipeline = &render_pass->pipelines_forward.back();
					pipeline->type = TYPE_TRANSPARENT;
					pipeline->reference.pipeline_type = TYPE_TRANSPARENT;
					pipeline->reference.renderpass = 0;
					pipeline->reference.pipeline = render_pass->pipelines_forward.size() - 1;
				}
			}
			state = SHADER_JSON_MAIN;
		}
		else if (state == SHADER_JSON_PROPERTY_NAME) {
			paramText = str;
			state = SHADER_JSON_PROPERTY;
		}
		else if (state == SHADER_JSON_PROPERTY_TYPE) {
			state = SHADER_JSON_PROPERTY;
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
			state = SHADER_JSON_PROPERTY;
			if (paramType == PARAM_TEXTURE || paramType == PARAM_CUBEMAP)
				paramDefault = (void *)str;
		}
		else if (state == SHADER_JSON_VERTEX) {
			if (shader_type == SHADER_JSON_SHADER_SHADOW) {
					pipeline->shadow_shader_paths[SHADER_VERTEX] = dir + str;
			}
			else {
				pipeline->shader_paths[SHADER_VERTEX] = dir + str;
			}
			if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
				state = SHADER_JSON_SHADER_DIRECTX;
			}
			else {
				state = SHADER_JSON_SHADER_VULKAN;
			}
		}
		else if (state == SHADER_JSON_FRAGMENT) {
			if (shader_type == SHADER_JSON_SHADER_SHADOW) {
				pipeline->shadow_shader_paths[SHADER_FRAGMENT] = dir + str;
			}
			else {
				pipeline->shader_paths[SHADER_FRAGMENT] = dir + str;
			}
			if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
				state = SHADER_JSON_SHADER_DIRECTX;
			}
			else {
				state = SHADER_JSON_SHADER_VULKAN;
			}
		}
		else if (state == SHADER_JSON_GEOMETRY) {
			if (shader_type == SHADER_JSON_SHADER_SHADOW) {
				pipeline->shadow_shader_paths[SHADER_GEOMETRY] = dir + str;
			}
			else {
				pipeline->shader_paths[SHADER_GEOMETRY] = dir + str;
			}
			if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
				state = SHADER_JSON_SHADER_DIRECTX;
			}
			else {
				state = SHADER_JSON_SHADER_VULKAN;
			}
		}
		else if (state == SHADER_JSON_COMPUTE) {
			std::cout << "Computer shaders not supported yet!" << std::endl;
			if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
				state = SHADER_JSON_SHADER_DIRECTX;
			}
			else {
				state = SHADER_JSON_SHADER_VULKAN;
			}
		}
		else if (state == SHADER_JSON_TESSEVAL) {
			if (shader_type == SHADER_JSON_SHADER_SHADOW) {
				pipeline->shadow_shader_paths[SHADER_TESS_EVALUATION] = dir + str;
			}
			else {
				pipeline->shader_paths[SHADER_TESS_EVALUATION] = dir + str;
			}
			state = SHADER_JSON_SHADER_OPENGL;
			if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
				state = SHADER_JSON_SHADER_DIRECTX;
			}
			else {
				state = SHADER_JSON_SHADER_VULKAN;
			}
		}
		else if (state == SHADER_JSON_TESSCTRL) {
			if (shader_type == SHADER_JSON_SHADER_SHADOW) {
				pipeline->shadow_shader_paths[SHADER_TESS_CONTROL] = dir + str;
			}
			else {
				pipeline->shader_paths[SHADER_TESS_CONTROL] = dir + str;
			}
			if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
				state = SHADER_JSON_SHADER_DIRECTX;
			}
			else {
				state = SHADER_JSON_SHADER_VULKAN;
			}
		}
		return true;
	}
	bool StartObject() {
		return true;
	}
	bool Key(const char* str, rapidjson::SizeType length, bool copy) {
		if (state == SHADER_JSON_MAIN) {
			if (std::string(str) == "name") {
				state = SHADER_JSON_NAME;
			}
			else if (std::string(str) == "type") {
				state = SHADER_JSON_TYPE;
			}
			else if (std::string(str) == "shaders") {
				state = SHADER_JSON_SHADERS;
			}
			else if (std::string(str) == "properties") {
				state = SHADER_JSON_PROPERTIES;
				in_textures = false;
			}
			else if (std::string(str) == "textures") {
				state = SHADER_JSON_PROPERTIES;
				in_textures = true;
			}
		}
		else if (state == SHADER_JSON_SHADERS) {
			if (std::string(str) == "deferred") {
				state = shader_type = SHADER_JSON_SHADER_DEFERRED;
			}
			else if (std::string(str) == "forward") {
				state = shader_type = SHADER_JSON_SHADER_FORWARD;
			}
			else if (std::string(str) == "shadow") {
				state = shader_type = SHADER_JSON_SHADER_SHADOW;
			}
			else {
				std::cout << "Invalid Shader file!" << std::endl;
			}
		}
		else if (state == SHADER_JSON_SHADER_DEFERRED || state == SHADER_JSON_SHADER_FORWARD || state == SHADER_JSON_SHADER_SHADOW) {
			if (std::string(str) == "opengl") {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (std::string(str) == "directx") {
				state = SHADER_JSON_SHADER_DIRECTX;
			}
			else if (std::string(str) == "vulkan") {
				state = SHADER_JSON_SHADER_VULKAN;
			}
			else {
				std::cout << "Invalid Shader file!" << std::endl;
			}
		}
		else if ((engine.settings.graphicsLanguage == GRAPHICS_OPENGL && state == SHADER_JSON_SHADER_OPENGL) ||
				(engine.settings.graphicsLanguage == GRAPHICS_DIRECTX && state == SHADER_JSON_SHADER_DIRECTX) ||
				(engine.settings.graphicsLanguage == GRAPHICS_VULKAN && state == SHADER_JSON_SHADER_VULKAN)) {
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
			state = SHADER_JSON_PROPERTY;
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
		switch (state) {
		case SHADER_JSON_PROPERTIES:
			state = SHADER_JSON_MAIN;
			break;
		case SHADER_JSON_PROPERTY:
			state = SHADER_JSON_PROPERTIES;
			if (in_textures) {
				pipeline->textureDescriptorTable[paramName] = TextureParameterDescriptor(texture_id++, paramText, paramType, paramDefault);
			}
			else {
				pipeline->parameterDescriptorTable[paramName] = ParameterDescriptor(paramText, paramType, paramDefault);
			}
			break;
		case SHADER_JSON_SHADER_DEFERRED:
		case SHADER_JSON_SHADER_FORWARD:
			state = SHADER_JSON_SHADERS;
			break;
		case SHADER_JSON_SHADER_OPENGL:
		case SHADER_JSON_SHADER_DIRECTX:
		case SHADER_JSON_SHADER_VULKAN:
			if (staterendertype == STATE_JSON_RENDER_DEFERRED)
				state = SHADER_JSON_SHADER_DEFERRED;
			else
				state = SHADER_JSON_SHADER_FORWARD;
			break;
		case SHADER_JSON_SHADERS:
			state = SHADER_JSON_MAIN;
			break;
		}

		return true;
	}
	bool StartArray() {
		subIterator = 0;
		return true;
	}
	bool EndArray(rapidjson::SizeType elementCount) {
		state = SHADER_JSON_PROPERTY;
		return true;
	}
}; 
#ifndef _MSC_VER
	typedef uint32_t DWORD;
#endif

struct DDS_PIXELFORMAT {
	DWORD dwSize = 32;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
};

struct DDSHeader {
	DWORD           dwSize = 124;
	DWORD           dwFlags;
	DWORD           dwHeight;
	DWORD           dwWidth;
	DWORD           dwPitchOrLinearSize;
	DWORD           dwDepth;
	DWORD           dwMipMapCount;
	DWORD           dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD           dwCaps;
	DWORD           dwCaps2;
	DWORD           dwCaps3;
	DWORD           dwCaps4;
	DWORD           dwReserved2;
};

#define DDPF_ALPHAPIXELS 0x1

#define MAKEFOURCC(c0, c1, c2, c3)	((DWORD)(char)(c0) | ((DWORD)(char)(c1) << 8) | \
									((DWORD)(char)(c2) << 16) | ((DWORD)(char)(c3) << 24))
#define MAKEFOURCCS(str)			MAKEFOURCC(str[0], str[1], str[2], str[3])
#define FOURCC_DXT1 MAKEFOURCC('D', 'X', 'T', '1')
#define FOURCC_DXT3 MAKEFOURCC('D', 'X', 'T', '3')
#define FOURCC_DXT5 MAKEFOURCC('D', 'X', 'T', '5')

ParameterDescriptor::ParameterDescriptor() {}
ParameterDescriptor::ParameterDescriptor(std::string _desc, PARAM_TYPE _type, void * _ptr) {
	description = _desc;
	paramType = _type;
	dataPtr = _ptr;
}

TextureParameterDescriptor::TextureParameterDescriptor(unsigned int _texture_id, std::string _desc, PARAM_TYPE _type, void * _ptr) {
	texture_id = _texture_id;
	description = _desc;
	paramType = _type;
	dataPtr = _ptr;
}

bool readFile(const std::string& filename, std::vector<char>& outputfile) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
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

RenderPassContainer *MaterialManager::Initialize(GraphicsWrapper *graphics_wrapper) {
	graphics_wrapper_ = graphics_wrapper;

	render_passes_.resize(1);

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
	render_passes_[0].renderPass = graphics_wrapper_->CreateRenderPass(rpci);

	/*CommandBufferCreateInfo cbci;
	cbci.steps = nullptr;
	cbci.count = 0;
	cbci.numOutputs = fboCount;
	cbci.secondaryInfo.isSecondary = false;
	render_passes_[0].commandBuffer = graphics_wrapper_->CreateCommandBuffer(cbci);

	cbci.steps = nullptr;
	cbci.count = 0;
	cbci.numOutputs = 1;
	cbci.secondaryInfo.isSecondary = true;
	cbci.secondaryInfo.fbos = render_passes_[0].framebuffers.data();
	cbci.secondaryInfo.fboCount = (uint32_t)render_passes_[0].framebuffers.size();
	cbci.secondaryInfo.renderPass = render_passes_[0].renderPass;
	pipeline->commandBuffer = graphics_wrapper_->CreateCommandBuffer(cbci);*/

	return nullptr;
}

void MaterialManager::generateProgram(GeometryInfo geometry_info, PipelineContainer &container) {
	{
		GraphicsPipelineCreateInfo gpci;
		gpci.scissorW = engine.settings.resolutionX;
		gpci.width = static_cast<float>(engine.settings.resolutionX);
		gpci.scissorH = engine.settings.resolutionY;
		gpci.height = static_cast<float>(engine.settings.resolutionY);
		gpci.renderPass = render_passes_[0].renderPass;
		gpci.attributes = geometry_info.vads;
		gpci.attributesCount = geometry_info.vads_count;
		gpci.bindings = geometry_info.vbds;
		gpci.bindingsCount = geometry_info.vbds_count;

		int num_shaders = 0;
		const int max_shaders = 5;
		for (int i = 0; i < max_shaders; i++) {
			if (container.shader_paths[i] != "") {
				++num_shaders;
			}
		}

		std::vector<ShaderStageCreateInfo> stages;
		stages.resize(num_shaders);
		std::vector<char> files[max_shaders];

		num_shaders = 0;
		for (int i = 0; i < max_shaders; i++) {
			if (container.shader_paths[i] != "") {
				stages[num_shaders].fileName = container.shader_paths[i].c_str();
				if (!readFile(stages[num_shaders].fileName, files[num_shaders])) {
					std::string warn = "Unable to load Shader!: " + std::string(stages[num_shaders].fileName) + "\n";
					throw std::runtime_error(warn);
				}

				stages[num_shaders].content = files[num_shaders].data();
				stages[num_shaders].size = (uint32_t)files[num_shaders].size();
				stages[num_shaders++].type = static_cast<ShaderStageType>(i);
			}
		}

		std::vector<TextureSubBinding> bindings;
		bindings.reserve(container.textureDescriptorTable.size());
		for (auto &t : container.textureDescriptorTable) {
			bindings.emplace_back(t.first.c_str(), t.second.texture_id);
		}

		int tbl_count;
		if (bindings.size() > 0) {
			TextureBindingLayoutCreateInfo tblci;
			tblci.bindingLocation = 2;
			tblci.bindings = bindings.data();
			tblci.bindingCount = (uint32_t)bindings.size();
			tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
			container.tbl = graphics_wrapper_->CreateTextureBindingLayout(tblci);
			tbl_count = 1;
		}
		else {
			container.tbl = nullptr;
			tbl_count = 0;
		}

		gpci.shaderStageCreateInfos = stages.data();
		gpci.shaderStageCreateInfoCount = (uint32_t)stages.size();
		gpci.uniformBufferBindings = geometry_info.ubbs;
		gpci.uniformBufferBindingCount = geometry_info.ubb_count;
		gpci.textureBindings = &container.tbl;
		gpci.textureBindingCount = tbl_count;
		gpci.cullMode = CULL_BACK;
		gpci.primitiveType = PRIM_TRIANGLES;
		container.program = graphics_wrapper_->CreateGraphicsPipeline(gpci);
	}
	{
		int num_shaders = 0;
		const int max_shaders = 5;
		for (int i = 0; i < max_shaders; i++) {
			if (container.shadow_shader_paths[i] != "") {
				++num_shaders;
			}
		}

		if (num_shaders > 0) {
			GraphicsPipelineCreateInfo gpci;
			gpci.scissorW = 1024;
			gpci.width = static_cast<float>(gpci.scissorW);
			gpci.scissorH = 1024;
			gpci.height = static_cast<float>(gpci.scissorH);
			gpci.renderPass = render_passes_[0].renderPass;
			gpci.attributes = geometry_info.vads;
			gpci.attributesCount = geometry_info.vads_count;
			gpci.bindings = geometry_info.vbds;
			gpci.bindingsCount = geometry_info.vbds_count;
			
			std::vector<ShaderStageCreateInfo> stages;
			stages.resize(num_shaders);
			std::vector<char> files[max_shaders];

			num_shaders = 0;
			for (int i = 0; i < max_shaders; i++) {
				if (container.shadow_shader_paths[i] != "") {
					stages[num_shaders].fileName = container.shadow_shader_paths[i].c_str();
					if (!readFile(stages[num_shaders].fileName, files[num_shaders])) {
						std::string warn = "Unable to load Shader!: " + std::string(stages[num_shaders].fileName) + "\n";
						throw std::runtime_error(warn);
					}

					stages[num_shaders].content = files[num_shaders].data();
					stages[num_shaders].size = (uint32_t)files[num_shaders].size();
					stages[num_shaders++].type = static_cast<ShaderStageType>(i);
				}
			}

			std::vector<TextureSubBinding> bindings;
			bindings.reserve(container.textureDescriptorTable.size());
			for (auto &t : container.textureDescriptorTable) {
				bindings.emplace_back(t.first.c_str(), t.second.texture_id);
			}

			int tbl_count;
			if (bindings.size() > 0) {
				TextureBindingLayoutCreateInfo tblci;
				tblci.bindingLocation = 2;
				tblci.bindings = bindings.data();
				tblci.bindingCount = (uint32_t)bindings.size();
				tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
				container.tbl = graphics_wrapper_->CreateTextureBindingLayout(tblci);
				tbl_count = 1;
			}
			else {
				container.tbl = nullptr;
				tbl_count = 0;
			}

			gpci.shaderStageCreateInfos = stages.data();
			gpci.shaderStageCreateInfoCount = (uint32_t)stages.size();
			gpci.uniformBufferBindings = geometry_info.ubbs;
			gpci.uniformBufferBindingCount = geometry_info.ubb_count;
			gpci.textureBindings = &container.tbl;
			gpci.textureBindingCount = tbl_count;
			gpci.cullMode = CULL_BACK;
			gpci.primitiveType = PRIM_TRIANGLES;
			container.shadow_program = graphics_wrapper_->CreateGraphicsPipeline(gpci);
		}
	}
}

PipelineReference MaterialManager::CreatePipeline(GeometryInfo geometry_info, std::string pipelineName, bool miscPipeline) {
	if (pipeline_map_.find(pipelineName) != pipeline_map_.end())
		return pipeline_map_[pipelineName];

	PipelineContainer *pipeline = nullptr;
	if (miscPipeline) {
		render_passes_[0].pipelines_misc.push_back(PipelineContainer());
		pipeline = &render_passes_[0].pipelines_misc.back();
		pipeline->type = TYPE_MISC;
		pipeline->reference.pipeline_type = TYPE_MISC;
		pipeline->reference.renderpass = 0;
		pipeline->reference.pipeline = render_passes_[0].pipelines_misc.size() - 1;
	}

	rapidjson::Reader reader;
	ShaderJSONHandler handler;
	handler.pipeline = pipeline;
	handler.misc = miscPipeline;
	handler.path = pipelineName;
	handler.dir = pipelineName.substr(0, pipelineName.find_last_of("/") + 1);
	handler.render_pass = &render_passes_[0];
	
	std::ifstream input(pipelineName);
	if (input.fail()) {
		std::string warn = "Input failed for " + pipelineName + "\nError: " + strerror(errno) + "\n";
		throw std::runtime_error(warn);
	}
	else {
		rapidjson::IStreamWrapper isw(input);
		reader.Parse(isw, handler);
		input.close();

		pipeline = handler.pipeline;
	}

	generateProgram(geometry_info, *pipeline);
	pipeline_map_[pipelineName] = pipeline->reference;

	// Create It
	return pipeline->reference;
}

void MaterialManager::LoadPreloaded() {
}

MaterialReference MaterialManager::PreLoadMaterial(GeometryInfo geometry_info, std::string path) {
	/*if (material_map_.find(path) != material_map_.end())
		return material_map_[path];

	std::ifstream input(path);
	if (!input.is_open()) {
		std::cerr << "Failed to open material: " << path.c_str() << "!\n";
		return MaterialReference();
	}

	if (engine.settings.showMaterialLoad)
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
		textureBinding = graphics_wrapper_->CreateTextureBinding(ci);
	}

	PipelineReference pipelineRef = CreatePipeline(geometry_info, shader);
	PipelineContainer *pipeline = GetPipeline(pipelineRef);

	MaterialReference ref;
	ref.pipelineReference = pipelineRef;
	ref.material = (uint32_t)pipeline->materials.size();

	pipeline->materials.emplace_back(ref, textureBinding);

	material_map_[path] = ref;
	return ref;*/
	return MaterialReference();
}

Texture *MaterialManager::LoadCubemap(std::string path) {
	/*if (texture_map_[path]) {
		if (engine.settings.showTextureLoad)
			printf("Texture reused: %s \n", path.c_str());
		return texture_map_[path];
	}*/

	const char *filecode = path.c_str() + path.size() - 3;

	if (strncmp(filecode, "dds", 3) == 0) {
		// DDS
		FILE *fp;

		fp = fopen(path.c_str(), "rb");
		if (fp == NULL) {
			std::string warn = "Cubemap failed to load: " + path + " \n";
			throw std::runtime_error(warn);
		}

		// Verify file code in header
		char filecode[4];
		fread(filecode, 1, 4, fp);
		if (strncmp(filecode, "DDS ", 4) != 0) {
			fclose(fp);
			std::string warn = "Invalid DDS cubemap: " + path + " \n";
			throw std::runtime_error(warn);
		}

		DDSHeader header;

		fread(&header, 124, 1, fp);

		unsigned char * buffer;
		unsigned int bufsize;
		bufsize = header.dwMipMapCount > 1 ? header.dwPitchOrLinearSize * 2 : header.dwPitchOrLinearSize;
		bufsize *= 6;
		buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
		fread(buffer, 1, bufsize, fp);

		fclose(fp);

		bool alphaflag = header.ddspf.dwFlags & DDPF_ALPHAPIXELS;
		if (alphaflag)
			G_WARNING(path + "\n");

		unsigned int components = (header.ddspf.dwFourCC == FOURCC_DXT1) ? 3 : 4;
		ColorFormat format;
		switch (header.ddspf.dwFourCC) {
		case FOURCC_DXT1:
			format = alphaflag ? FORMAT_COLOR_RGBA_DXT1 : FORMAT_COLOR_RGB_DXT1;
			break;
		case FOURCC_DXT3:
			format = FORMAT_COLOR_RGBA_DXT3;
			break;
		case FOURCC_DXT5:
			format = FORMAT_COLOR_RGBA_DXT5;
			break;
		default:
			free(buffer);
			std::string warn = "Invalid FourCC in cubemap: " + path + " \n";
			throw std::runtime_error(warn);
		}

		TextureCreateInfo createInfo;
		createInfo.data = buffer;
		createInfo.mipmaps = header.dwMipMapCount;
		createInfo.format = format;
		createInfo.width = header.dwWidth;
		createInfo.height = header.dwHeight;
		createInfo.ddscube = true;

		Texture *t = graphics_wrapper_->CreateTexture(createInfo);
		texture_map_[path] = t;

		if (engine.settings.showTextureLoad)
			printf("Cubemap loaded: %s \n", path.c_str());

		stbi_image_free(buffer);

		return t;
	}
	else {
		int d = path.find_last_of('.');
		std::string ext = path.substr(d + 1);
		path = path.substr(0, d);

		std::string facePaths[6];
		facePaths[0] = path + "ft." + ext;
		facePaths[1] = path + "bk." + ext;
		facePaths[2] = path + "up." + ext;
		facePaths[3] = path + "dn." + ext;
		facePaths[4] = path + "rt." + ext;
		facePaths[5] = path + "lf." + ext;


		CubemapCreateInfo createInfo;

		int texWidth, texHeight, texChannels;
		for (int i = 0; i < 6; i++) {
			createInfo.data[i] = stbi_load(facePaths[i].c_str(), &texWidth, &texHeight, &texChannels, 4);
			if (!createInfo.data[i]) {
				for (int j = 0; j < i; j++) {
					stbi_image_free(createInfo.data[j]);
				}
				std::string warn = "Texture failed to load!: " + facePaths[i] + " \n";
				throw std::runtime_error(warn);
				return NULL;
			}
		}

		G_NOTIFY("Cubemap loaded: %s \n", path.c_str());

		ColorFormat format;
		switch (4) {
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

		createInfo.format = format;
		createInfo.mipmaps = 0;
		createInfo.width = texWidth;
		createInfo.height = texHeight;

		Texture *t = graphics_wrapper_->CreateCubemap(createInfo);

		for (int i = 0; i < 6; i++) {
			stbi_image_free(createInfo.data[i]);
		}

		return t;
	}
}

std::istream& safeGetline(std::istream& is, std::string& t)
{
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    for(;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if(sb->sgetc() == '\n')
                sb->sbumpc();
            return is;
        case std::streambuf::traits_type::eof():
            // Also handle the case when the last line has no line ending
            if(t.empty())
                is.setstate(std::ios::eofbit);
            return is;
        default:
            t += (char)c;
        }
    }
}

MaterialReference MaterialManager::CreateMaterial(GeometryInfo geometry_info, std::string path, bool miscPipeline) {
	if (material_map_.find(path) != material_map_.end()) {
		return material_map_[path];
	}

	std::ifstream input(path);
	if (!input.is_open()) {
		std::string warning = std::string("Material failed to load: ") + path.c_str() + " \n";
		throw std::runtime_error(warning.c_str());
	}

	if (engine.settings.showMaterialLoad)
		std::cout << "Material reading from: " << path << "!\n";

	std::string shader_param;
	safeGetline(input, shader_param);
	unsigned int p = shader_param.find(':');
	if (p == -1) {
		throw std::runtime_error("Error! " + path + " is invalid, the first line must refer to the shader.");
	}
	else {
		if (shader_param.substr(0, p) != "shader") {
			throw std::runtime_error("Error! " + path + " is invalid, the first line must refer to the shader.");
		}
		shader_param = shader_param.substr(p+2);
	}

	PipelineReference pipeline_reference = CreatePipeline(geometry_info, shader_param, miscPipeline);
	PipelineContainer *pipeline = GetPipeline(pipeline_reference);

	std::vector<SingleTextureBind> textures;
	textures.resize(pipeline->textureDescriptorTable.size());
	std::string dir = path.substr(0, path.find_last_of("/") + 1);

	std::string line, parameter, value;
	while (safeGetline(input, line)) {
		p = line.find(':');
		if (p != -1) {
			parameter = line.substr(0, p);
			value = line.substr(p+2);
			auto it1 = pipeline->parameterDescriptorTable.find(parameter);
			if (it1 != pipeline->parameterDescriptorTable.end()) {
				G_NOTIFY("Found Parameter %s:%s\n", parameter, value);
			}
			else {
				auto it2 = pipeline->textureDescriptorTable.find(parameter);
				if (it2 != pipeline->textureDescriptorTable.end()) {
					unsigned int texture_id = it2->second.texture_id;
					if (it2->second.paramType == PARAM_CUBEMAP) {
						textures[texture_id].texture = LoadCubemap(dir + value);
					}
					else {
						textures[texture_id].texture = LoadTexture(dir + value);
					}
					textures[texture_id].address = texture_id;
				}
				else {
					G_WARNING("Invalid parameter %s.\n", parameter);
				}
			}
		}
	}

	TextureBinding *textureBinding = nullptr;
	if (textures.size() > 0) {
		//if (settings.engine.showMaterialLoad)
		//	std::cout << path << " has " << textures.size() << " textures." << std::endl;

		TextureBindingCreateInfo ci;
		ci.layout = pipeline->tbl;
		ci.textures = textures.data();
		ci.textureCount = (uint32_t)textures.size();
		textureBinding = graphics_wrapper_->CreateTextureBinding(ci);
	}
	
	MaterialReference ref;
	ref.pipelineReference = pipeline_reference;
	ref.material = (uint32_t)pipeline->materials.size();

	pipeline->materials.emplace_back(ref, textureBinding);

	material_map_[path] = ref;
	return ref;
}

RenderPassContainer *MaterialManager::GetRenderPass(uint8_t i) {
	return &render_passes_[i];
}

PipelineContainer * MaterialManager::GetPipeline(PipelineReference ref) {
	if (ref.pipeline_type == TYPE_UNLIT)
		return &render_passes_[ref.renderpass].pipelines_unlit[ref.pipeline];
	else if (ref.pipeline_type == TYPE_OPAQUE)
		return &render_passes_[ref.renderpass].pipelines_deferred[ref.pipeline];
	else if (ref.pipeline_type == TYPE_MISC)
		return &render_passes_[ref.renderpass].pipelines_misc[ref.pipeline];
	else
		return &render_passes_[ref.renderpass].pipelines_forward[ref.pipeline];
}

Material * MaterialManager::GetMaterial(MaterialReference ref) {
	if (ref.pipelineReference.pipeline_type == TYPE_UNLIT)
		return &render_passes_[ref.pipelineReference.renderpass].pipelines_unlit[ref.pipelineReference.pipeline].materials[ref.material];
	else if (ref.pipelineReference.pipeline_type == TYPE_OPAQUE)
		return &render_passes_[ref.pipelineReference.renderpass].pipelines_deferred[ref.pipelineReference.pipeline].materials[ref.material];
	else if (ref.pipelineReference.pipeline_type == TYPE_MISC)
		return &render_passes_[ref.pipelineReference.renderpass].pipelines_misc[ref.pipelineReference.pipeline].materials[ref.material];
	else
		return &render_passes_[ref.pipelineReference.renderpass].pipelines_forward[ref.pipelineReference.pipeline].materials[ref.material];
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

Texture *MaterialManager::PreLoadTexture(std::string path) {
	if (texture_map_[path]) {
		if (engine.settings.showTextureLoad)
			printf("Texture reused: %s \n", path.c_str());
		return texture_map_[path];
	}

	//unloaded_.push_back();
	return nullptr;
}

Texture * MaterialManager::LoadTexture(std::string path) {
	if (texture_map_[path]) {
		if (engine.settings.showTextureLoad)
			printf("Texture reused: %s \n", path.c_str());
		return texture_map_[path];
	}

	const char *filecode = path.c_str() + path.size() - 3;
	if (strncmp(filecode, "dds", 3) == 0) {
		// DDS
		FILE *fp;

		fp = fopen(path.c_str(), "rb");
		if (fp == NULL) {
			printf("Texture failed to load!: %s \n", path.c_str());
			return 0;
		}

		// Verify file code in header
		char filecode[4];
		fread(filecode, 1, 4, fp);
		if (strncmp(filecode, "DDS ", 4) != 0) {
			printf("Texture failed to load!: %s \n", path.c_str());
			fclose(fp);
			return 0;
		}

		DDSHeader header;

		fread(&header, 124, 1, fp);

		unsigned char * buffer;
		unsigned int bufsize;
		bufsize = header.dwMipMapCount > 1 ? header.dwPitchOrLinearSize * 2 : header.dwPitchOrLinearSize;
		buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
		fread(buffer, 1, bufsize, fp);

		fclose(fp);

		bool alphaflag = header.ddspf.dwFlags & DDPF_ALPHAPIXELS;
		if (alphaflag)
			G_WARNING(path + "\n");

		unsigned int components = (header.ddspf.dwFourCC == FOURCC_DXT1) ? 3 : 4;
		ColorFormat format;
		switch (header.ddspf.dwFourCC) {
		case FOURCC_DXT1:
			format = alphaflag ? FORMAT_COLOR_RGBA_DXT1 : FORMAT_COLOR_RGB_DXT1;
			break;
		case FOURCC_DXT3:
			format = FORMAT_COLOR_RGBA_DXT3;
			break;
		case FOURCC_DXT5:
			format = FORMAT_COLOR_RGBA_DXT5;
			break;
		default:
			free(buffer);
			printf("Invalid FourCC in texture: %s \n", path.c_str());
			return 0;
		}

		TextureCreateInfo createInfo;
		createInfo.data = buffer;
		createInfo.mipmaps = header.dwMipMapCount;
		createInfo.format = format;
		createInfo.width = header.dwWidth;
		createInfo.height = header.dwHeight;
		createInfo.ddscube = false;

		Texture *t = graphics_wrapper_->CreateTexture(createInfo);
		texture_map_[path] = t;

		if (engine.settings.showTextureLoad)
			printf("Texture loaded: %s \n", path.c_str());

		stbi_image_free(buffer);

		return t;
	}
	else {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		texChannels = 4;
		if (!pixels) {
			std::string warning = std::string("Texture failed to load: ") + path.c_str() + " \n";
			throw std::runtime_error(warning.c_str());
			return NULL;
		}

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
		createInfo.ddscube = false;

		Texture *t = graphics_wrapper_->CreateTexture(createInfo);
		texture_map_[path] = t;

		stbi_image_free(pixels);

		if (engine.settings.showTextureLoad)
			printf("Texture loaded: %s \n", path.c_str());

		return t;	
	}
	return nullptr;
}

void MaterialManager::DrawUnlitImmediate() {
	for (auto const &renderPass : render_passes_) {
		for (auto const &pipeline : renderPass.pipelines_unlit) {
			pipeline.program->Bind();
			if (pipeline.draw_count > 0) {
				for (auto const &material : pipeline.materials) {
					if (material.draw_count > 0) {
						if (material.m_textureBinding != nullptr)
							graphics_wrapper_->BindTextureBinding(material.m_textureBinding);
						for (auto const &mesh : material.m_meshes) {
							mesh->Draw();
						}
					}
				}
			}
		}
	}
}

void MaterialManager::DrawShadowsImmediate() {
	for (auto const &renderPass : render_passes_) {
		for (auto const &pipeline : renderPass.pipelines_deferred) {
			if (pipeline.shadow_program != nullptr) {
				pipeline.shadow_program->Bind();
				if (pipeline.draw_count > 0) {
					for (auto const &material : pipeline.materials) {
						if (material.draw_count > 0) {
							if (material.m_textureBinding != nullptr)
								graphics_wrapper_->BindTextureBinding(material.m_textureBinding);
							for (auto const &mesh : material.m_meshes) {
								mesh->ShadowDraw();
							}
						}
					}
				}
			}
		}

		for (auto const &pipeline : renderPass.pipelines_unlit) {
			if (pipeline.shadow_program != nullptr) {
				pipeline.shadow_program->Bind();
				if (pipeline.draw_count > 0) {
					for (auto const &material : pipeline.materials) {
						if (material.draw_count > 0) {
							if (material.m_textureBinding != nullptr)
								graphics_wrapper_->BindTextureBinding(material.m_textureBinding);
							for (auto const &mesh : material.m_meshes) {
								mesh->ShadowDraw();
							}
						}
					}
				}
			}
		}
	}
}

void MaterialManager::DrawDeferredImmediate() {
	for (auto const &renderPass : render_passes_) {
		//renderPass->Bind();
		for (auto const &pipeline : renderPass.pipelines_deferred) {
			pipeline.program->Bind();
			if (pipeline.draw_count > 0) {
				for (auto const &material : pipeline.materials) {
					if (material.draw_count > 0) {
						if (material.m_textureBinding != nullptr)
							graphics_wrapper_->BindTextureBinding(material.m_textureBinding);
						for (auto const &mesh : material.m_meshes) {
							mesh->Draw();
						}
					}
				}
			}
		}
	}
}

void MaterialManager::DrawForwardImmediate() {
	for (auto const &renderPass : render_passes_) {
		for (auto const &pipeline : renderPass.pipelines_forward) {
			pipeline.program->Bind();
			if (pipeline.draw_count > 0) {
				for (auto const &material : pipeline.materials) {
					if (material.draw_count > 0) {
						for (auto const &mesh : material.m_meshes) {
							mesh->Draw();
						}
					}
				}
			}
		}
	}
}

void MaterialManager::DrawDeferredCommand() {
	uint32_t currentFrame = graphics_wrapper_->GetImageIndex();

	for (auto const &renderPass : render_passes_) {
		std::vector<CommandBuffer *> cmds;
		cmds.reserve(renderPass.pipelines_deferred.size());
		for (auto const &pipeline : renderPass.pipelines_deferred) {
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

void MaterialManager::resetDraws() {
	for (auto &render_pass : render_passes_) {
		for (auto &pipeline : render_pass.pipelines_deferred) {
			pipeline.draw_count = 1;
			for (auto &material : pipeline.materials) {
				material.draw_count = 1;
			}
		}
		for (auto &pipeline : render_pass.pipelines_unlit) {
			pipeline.draw_count = 1;
			for (auto &material : pipeline.materials) {
				material.draw_count = 1;
			}
		}
		for (auto &pipeline : render_pass.pipelines_forward) {
			pipeline.draw_count = 1;
			for (auto &material : pipeline.materials) {
				material.draw_count = 1;
			}
		}
	}
}

MaterialManager::~MaterialManager() {
	for (const auto &texture : texture_map_) {
		graphics_wrapper_->DeleteTexture(texture.second);
	}

	for (const auto &render_pass : render_passes_) {
		for (const auto &pipeline : render_pass.pipelines_deferred) {
			graphics_wrapper_->DeleteGraphicsPipeline(pipeline.program);
		}

		for (const auto &pipeline : render_pass.pipelines_unlit) {
			graphics_wrapper_->DeleteGraphicsPipeline(pipeline.program);
		}

		for (const auto &pipeline : render_pass.pipelines_misc) {
			graphics_wrapper_->DeleteGraphicsPipeline(pipeline.program);
		}

		for (const auto &pipeline : render_pass.pipelines_forward) {
			graphics_wrapper_->DeleteGraphicsPipeline(pipeline.program);
		}

		graphics_wrapper_->DeleteRenderPass(render_pass.renderPass);
	}
}

Material::Material(MaterialReference reference, TextureBinding * textureBinding) {
	m_textureBinding = textureBinding;
	this->reference = reference;
}

void Material::IncrementDrawCount() {
	draw_count++;
	engine.materialManager.GetPipeline(reference.pipelineReference)->draw_count++;
}
