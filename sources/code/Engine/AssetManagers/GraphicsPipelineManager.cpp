// STD headers
#include <fstream>

// My Class
#include "GraphicsPipelineManager.hpp"

// Included Classes
#include "ModelManager.hpp"
#include "TextureManager.hpp"
#include "Core/Engine.hpp"
#include <GraphicsWrapper.hpp>
#include <Texture.hpp>
#include <GraphicsPipeline.hpp>
#include <CommandBuffer.hpp>
#include <UniformBuffer.hpp>

// Util Classes
#include "../Utilities/Logger.hpp"
#include "Core/Utilities.hpp"
#include "glm/common.hpp"

// RapidJSON
#undef Bool
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/error/en.h"

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

	uint16_t param_size;

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
					pipeline->reference.pipeline = uint8_t(render_pass->pipelines_unlit.size() - 1);
				}
				else if (type == "opaque") {
					render_pass->pipelines_deferred.push_back(PipelineContainer());
					pipeline = &render_pass->pipelines_deferred.back();
					pipeline->type = TYPE_OPAQUE;
					pipeline->reference.pipeline_type = TYPE_OPAQUE;
					pipeline->reference.renderpass = 0;
					pipeline->reference.pipeline = uint8_t(render_pass->pipelines_deferred.size() - 1);
				}
				else if (type == "transparent") {
					render_pass->pipelines_forward.push_back(PipelineContainer());
					pipeline = &render_pass->pipelines_forward.back();
					pipeline->type = TYPE_TRANSPARENT;
					pipeline->reference.pipeline_type = TYPE_TRANSPARENT;
					pipeline->reference.renderpass = 0;
					pipeline->reference.pipeline = uint8_t(render_pass->pipelines_forward.size() - 1);
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
			if (std::string(str) == "boolean") {
				paramType = PARAM_BOOL;
				param_size += 4;
			}
			else if (std::string(str) == "constant") {
				paramType = PARAM_FLOAT;
				param_size += sizeof(float);
			}
			else if (std::string(str) == "color") {
				paramType = PARAM_VEC4;
				param_size += sizeof(glm::vec4);
			}
			else if (std::string(str) == "int") {
				paramType = PARAM_INT;
				param_size += sizeof(int);
			}
			else if (std::string(str) == "uint") {
				paramType = PARAM_UINT;
				param_size += sizeof(unsigned int);
			}
			else if (std::string(str) == "float") {
				paramType = PARAM_FLOAT;
				param_size += sizeof(float);
			}
			else if (std::string(str) == "double") {
				paramType = PARAM_DOUBLE;
				param_size += sizeof(double);
			}
			else if (std::string(str) == "bvec2") {
				paramType = PARAM_BVEC2;
				param_size += 4;
			}
			else if (std::string(str) == "bvec3") {
				paramType = PARAM_BVEC3;
				param_size += 4;
			}
			else if (std::string(str) == "bvec4") {
				paramType = PARAM_BVEC4;
				param_size += 4;
			}
			else if (std::string(str) == "ivec2") {
				paramType = PARAM_IVEC2;
				param_size += sizeof(int) * 2;
			}
			else if (std::string(str) == "ivec3") {
				paramType = PARAM_IVEC3;
				param_size += sizeof(int) * 3;
			}
			else if (std::string(str) == "ivec4") {
				paramType = PARAM_IVEC4;
				param_size += sizeof(int) * 4;
			}
			else if (std::string(str) == "uvec2") {
				paramType = PARAM_UVEC2;
				param_size += sizeof(unsigned int) * 2;
			}
			else if (std::string(str) == "uvec3") {
				paramType = PARAM_UVEC3;
				param_size += sizeof(unsigned int) * 3;
			}
			else if (std::string(str) == "uvec4") {
				paramType = PARAM_UVEC4;
				param_size += sizeof(unsigned int) * 4;
			}
			else if (std::string(str) == "vec2") {
				paramType = PARAM_VEC2;
				param_size += sizeof(glm::vec2);
			}
			else if (std::string(str) == "vec3") {
				paramType = PARAM_VEC3;
				param_size += sizeof(glm::vec3);
			}
			else if (std::string(str) == "vec4") {
				paramType = PARAM_VEC4;
				param_size += sizeof(glm::vec4);
			}
			else if (std::string(str) == "dvec2") {
				paramType = PARAM_DVEC2;
				param_size += sizeof(double) * 2;
			}
			else if (std::string(str) == "dvec3") {
				paramType = PARAM_DVEC3;
				param_size += sizeof(double) * 3;
			}
			else if (std::string(str) == "dvec4") {
				paramType = PARAM_DVEC4;
				param_size += sizeof(double) * 4;
			}
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
			if (engine.getSettings()->graphics_language_ == GRAPHICS_OPENGL) {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (engine.getSettings()->graphics_language_ == GRAPHICS_DIRECTX) {
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
			if (engine.getSettings()->graphics_language_ == GRAPHICS_OPENGL) {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (engine.getSettings()->graphics_language_ == GRAPHICS_DIRECTX) {
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
			if (engine.getSettings()->graphics_language_ == GRAPHICS_OPENGL) {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (engine.getSettings()->graphics_language_ == GRAPHICS_DIRECTX) {
				state = SHADER_JSON_SHADER_DIRECTX;
			}
			else {
				state = SHADER_JSON_SHADER_VULKAN;
			}
		}
		else if (state == SHADER_JSON_COMPUTE) {
			GRIND_ERROR("Computer shaders not supported yet!");
			if (engine.getSettings()->graphics_language_ == GRAPHICS_OPENGL) {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (engine.getSettings()->graphics_language_ == GRAPHICS_DIRECTX) {
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
			if (engine.getSettings()->graphics_language_ == GRAPHICS_OPENGL) {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (engine.getSettings()->graphics_language_ == GRAPHICS_DIRECTX) {
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
			if (engine.getSettings()->graphics_language_ == GRAPHICS_OPENGL) {
				state = SHADER_JSON_SHADER_OPENGL;
			}
			else if (engine.getSettings()->graphics_language_ == GRAPHICS_DIRECTX) {
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
				param_size = 0;
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
				GRIND_ERROR("Invalid Shader file!");
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
				GRIND_ERROR("Invalid Shader file!");
			}
		}
		else if ((engine.getSettings()->graphics_language_ == GRAPHICS_OPENGL && state == SHADER_JSON_SHADER_OPENGL) ||
				(engine.getSettings()->graphics_language_ == GRAPHICS_DIRECTX && state == SHADER_JSON_SHADER_DIRECTX) ||
				(engine.getSettings()->graphics_language_ == GRAPHICS_VULKAN && state == SHADER_JSON_SHADER_VULKAN)) {
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
				pipeline->param_size = param_size;
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

void GraphicsPipelineManager::generateProgram(GeometryInfo geometry_info, PipelineContainer &container) {
	// Prepare UBBS
	std::vector<UniformBufferBinding *> ubbs;
	if (container.param_ubb != nullptr) {
		ubbs.resize(geometry_info.ubb_count + 1);
		ubbs[geometry_info.ubb_count] = container.param_ubb;
	}
	else {
		ubbs.resize(geometry_info.ubb_count);
	}

	for (int i = 0; i < geometry_info.ubb_count; ++i) {
		ubbs[i] = geometry_info.ubbs[i];
	} 
	

	// Create Regular Shader
	{
		GraphicsPipelineCreateInfo gpci;
		gpci.scissorW = engine.getSettings()->resolution_x_;
		gpci.width = static_cast<float>(engine.getSettings()->resolution_x_);
		gpci.scissorH = engine.getSettings()->resolution_y_;
		gpci.height = static_cast<float>(engine.getSettings()->resolution_y_);
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
			container.tbl = engine.getGraphicsWrapper()->CreateTextureBindingLayout(tblci);
			tbl_count = 1;
		}
		else {
			container.tbl = nullptr;
			tbl_count = 0;
		}

		gpci.shaderStageCreateInfos = stages.data();
		gpci.shaderStageCreateInfoCount = (uint32_t)stages.size();
		gpci.uniformBufferBindings = ubbs.data();
		gpci.uniformBufferBindingCount = ubbs.size();
		gpci.textureBindings = &container.tbl;
		gpci.textureBindingCount = tbl_count;
		gpci.cullMode = CULL_BACK;
		gpci.primitiveType = PRIM_TRIANGLES;
		container.program = engine.getGraphicsWrapper()->CreateGraphicsPipeline(gpci);
	}

	//  Create Shadows
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
				container.tbl = engine.getGraphicsWrapper()->CreateTextureBindingLayout(tblci);
				tbl_count = 1;
			}
			else {
				container.tbl = nullptr;
				tbl_count = 0;
			}
			
			gpci.shaderStageCreateInfos = stages.data();
			gpci.shaderStageCreateInfoCount = (uint32_t)stages.size();
			gpci.uniformBufferBindings = ubbs.data();
			gpci.uniformBufferBindingCount = ubbs.size();
			gpci.textureBindings = &container.tbl;
			gpci.textureBindingCount = tbl_count;
			gpci.cullMode = CULL_BACK;
			gpci.primitiveType = PRIM_TRIANGLES;
			container.shadow_program = engine.getGraphicsWrapper()->CreateGraphicsPipeline(gpci);
		}
	}
}

void GraphicsPipelineManager::resetDraws()
{
}

void GraphicsPipelineManager::cleanup()
{
}

GraphicsPipelineManager::~GraphicsPipelineManager()
{
}

GraphicsPipelineManager::GraphicsPipelineManager() {
	initialize();
}

void GraphicsPipelineManager::initialize() {
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
	rpci.m_width = engine.getSettings()->resolution_x_;
	rpci.m_height = engine.getSettings()->resolution_y_;
	rpci.m_depthFormat = FORMAT_DEPTH_32;
	render_passes_[0].renderPass = engine.getGraphicsWrapper()->CreateRenderPass(rpci);
}

PipelineReference GraphicsPipelineManager::createPipeline(GeometryInfo geometry_info, std::string pipelineName, bool miscPipeline) {
	if (pipeline_map_.find(pipelineName) != pipeline_map_.end())
		return pipeline_map_[pipelineName];

	PipelineContainer *pipeline = nullptr;
	if (miscPipeline) {
		render_passes_[0].pipelines_misc.push_back(PipelineContainer());
		pipeline = &render_passes_[0].pipelines_misc.back();
		pipeline->type = TYPE_MISC;
		pipeline->reference.pipeline_type = TYPE_MISC;
		pipeline->reference.renderpass = 0;
		pipeline->reference.pipeline = uint8_t(render_passes_[0].pipelines_misc.size() - 1);
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

	if (pipeline->parameterDescriptorTable.size() > 0) {
		UniformBufferBindingCreateInfo ubbci;
		ubbci.binding = 2;
		ubbci.shaderLocation = "Parameters";
		ubbci.size = pipeline->param_size;
		ubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
		pipeline->param_ubb = engine.getGraphicsWrapper()->CreateUniformBufferBinding(ubbci);
	}

	generateProgram(geometry_info, *pipeline);
	pipeline_map_[pipelineName] = pipeline->reference;

	// Create It
	return pipeline->reference;
}

RenderPassContainer * GraphicsPipelineManager::getRenderPass(uint8_t ref) {
	return &render_passes_[ref];
}

PipelineContainer * GraphicsPipelineManager::getPipeline(PipelineReference ref) {
	auto rp = getRenderPass(ref.renderpass);
	if (ref.pipeline_type == TYPE_UNLIT)
		return &rp->pipelines_unlit[ref.pipeline];
	else if (ref.pipeline_type == TYPE_OPAQUE)
		return &rp->pipelines_deferred[ref.pipeline];
	else if (ref.pipeline_type == TYPE_MISC)
		return &rp->pipelines_misc[ref.pipeline];
	else
		return &rp->pipelines_forward[ref.pipeline];
}

void GraphicsPipelineManager::refresh()
{
}

void GraphicsPipelineManager::removeRenderPass(uint8_t i)
{
}

void GraphicsPipelineManager::removePipeline(PipelineReference)
{
}

void GraphicsPipelineManager::loadPreloaded()
{
}

void GraphicsPipelineManager::drawUnlitImmediate() {
	for (auto &renderPass : render_passes_) {
		for (auto &pipeline : renderPass.pipelines_unlit) {
			pipeline.program->Bind();
			if (pipeline.draw_count > 0) {
				for (auto &material : pipeline.materials) {
					if (material.getDrawCount() > 0) {
						if (material.m_textureBinding != nullptr)
							engine.getGraphicsWrapper()->BindTextureBinding(material.m_textureBinding);
						for (auto mesh : material.m_meshes) {
							mesh->draw();
						}
					}
				}
			}
		}
	}
}

void GraphicsPipelineManager::drawShadowsImmediate(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	for (auto &renderPass : render_passes_) {
		for (auto &pipeline : renderPass.pipelines_deferred) {
			if (pipeline.shadow_program != nullptr) {
				pipeline.shadow_program->Bind();
				engine.getGraphicsWrapper()->setViewport(x, y, w, h);
				if (true || pipeline.draw_count > 0) {
					for (auto &material : pipeline.materials) {
						if (true || material.getDrawCount() > 0) {
							if (material.m_textureBinding != nullptr)
								engine.getGraphicsWrapper()->BindTextureBinding(material.m_textureBinding);
							for (auto mesh : material.m_meshes) {
								mesh->shadowDraw();
							}
						}
					}
				}
			}
		}

		for (auto &pipeline : renderPass.pipelines_unlit) {
			if (pipeline.shadow_program != nullptr) {
				pipeline.shadow_program->Bind();
				if (pipeline.draw_count > 0) {
					for (auto &material : pipeline.materials) {
						if (material.getDrawCount() > 0) {
							if (material.m_textureBinding != nullptr)
								engine.getGraphicsWrapper()->BindTextureBinding(material.m_textureBinding);
							for (auto mesh : material.m_meshes) {
								mesh->shadowDraw();
							}
						}
					}
				}
			}
		}
	}
}

void GraphicsPipelineManager::drawDeferredImmediate() {
	for (auto &renderPass : render_passes_) {
		//renderPass->Bind();
		for (auto &pipeline : renderPass.pipelines_deferred) {
			pipeline.program->Bind();
			if (true || pipeline.draw_count > 0) {
				for (auto &material : pipeline.materials) {
					if (material.getDrawCount() > 0) {
						if (material.m_textureBinding != nullptr)
							engine.getGraphicsWrapper()->BindTextureBinding(material.m_textureBinding);

						if (material.param_buffer_handler_ != nullptr)
							material.param_buffer_handler_->Bind();

						for (auto &mesh : material.m_meshes) {
							mesh->draw();
						}
					}
				}
			}
		}
	}
}

void GraphicsPipelineManager::drawForwardImmediate() {
	for (auto &renderPass : render_passes_) {
		for (auto &pipeline : renderPass.pipelines_forward) {
			pipeline.program->Bind();
			if (pipeline.draw_count > 0) {
				for (auto &material : pipeline.materials) {
					if (material.getDrawCount() > 0) {
						for (auto &mesh : material.m_meshes) {
							mesh->draw();
						}
					}
				}
			}
		}
	}
}
