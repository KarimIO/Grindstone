// STD headers
#include <fstream>

// My Class
#include "MaterialManager.hpp"

// Included Classes
#include "GraphicsPipelineManager.hpp"
#include "TextureManager.hpp"
#include "Core/Engine.hpp"
#include <GraphicsWrapper.hpp>
#include "AssetReferences.hpp"
#include "ModelManager.hpp"

// Util Classes
#include "../Utilities/Logger.hpp"
#include "Core/Utilities.hpp"

Material::Material(MaterialReference reference, TextureBinding * textureBinding) {
	m_textureBinding = textureBinding;
	this->reference = reference;
}

void Material::incrementDrawCount() {
	draw_count++;
	engine.getGraphicsPipelineManager()->getPipeline(reference.pipelineReference)->draw_count++;
}

const uint32_t Material::getDrawCount() {
	return draw_count;
}

MaterialReference MaterialManager::loadMaterial(GeometryInfo geometry_info, std::string path) {
	//  Check if we have a material with the same name
	if (material_map_.find(path) != material_map_.end()) {
		return material_map_[path];
	}

	// Load the material
	std::ifstream input(path);
	if (input.fail()) {
		GRIND_WARN("Material failed to load: {0}.", path);
		return empty_material_reference_;
	}

	if (engine.getSettings()->show_material_load_)
		GRIND_LOG("Material reading from: {0}!", path.c_str());

	// Get Shader Name
	std::string shader_param;
	safeGetline(input, shader_param);
	size_t p = shader_param.find(':');
	if (p == -1) {
		// Return empty if we can't find the delimeter
		GRIND_WARN("{0} is invalid, the first line must refer to the shader.", path);
		return empty_material_reference_;
	}
	else {
		if (shader_param.substr(0, p) != "shader") {
			// Return empty if the key isn't "shader"
			GRIND_WARN("{0} is invalid, the first line must refer to the shader.", path);
			return empty_material_reference_;
		}

		// Get Parameter Name
		shader_param = shader_param.substr(p+2);
	}

	// Load GraphicsPipeline
	auto pipeline_manager = engine.getGraphicsPipelineManager();
	PipelineReference pipeline_reference = pipeline_manager->createPipeline(geometry_info, shader_param);
	PipelineContainer *pipeline = pipeline_manager->getPipeline(pipeline_reference);

	// Create Table of Textures
	std::vector<SingleTextureBind> textures;
	textures.resize(pipeline->textureDescriptorTable.size());
	std::string dir = path.substr(0, path.find_last_of("/") + 1);

	char *buffer = new char[pipeline->param_size];
	memset(buffer, 0, pipeline->param_size);
	char *buffpos = buffer;

	// Load parameters
	auto texture_manager = engine.getTextureManager();
	std::string line, parameter, value;
	while (safeGetline(input, line)) {
		p = line.find(':'); // Find next parameter delimeter
		if (p != -1) { // If we don't find it on this line, skip the line
			parameter = line.substr(0, p);
			value = line.substr(p+2);
			auto it1 = pipeline->parameterDescriptorTable.find(parameter);
			if (it1 != pipeline->parameterDescriptorTable.end()) {
				// This is a parameter, and we don't handle those yet.
				//GRIND_LOG("In %s, found Parameter %s:%s\n", path, parameter, value);
				auto it2 = pipeline->parameterDescriptorTable.find(parameter);
				if (it2 != pipeline->parameterDescriptorTable.end()) {
					size_t pos = 0;
					if (it2->second.paramType == PARAM_VEC4) {
						float *list = (float *)buffpos;
						for (int i = 0; i < 4; ++i) {
							size_t npos = value.find(" ", pos);
							std::string floatval = value.substr(pos, npos - pos);
							pos = npos + 1;
							list[i] = std::stof(floatval);
						}

						buffpos += sizeof(float) * 4;
					}
					else if (it2->second.paramType == PARAM_FLOAT) {
						*(float *)buffpos = std::stof(value);
						buffpos += sizeof(float);
					}
					else if (it2->second.paramType == PARAM_BOOL) {
						*(bool *)buffpos = (value == "true") ? true : false;
						buffpos += sizeof(bool) * 4;
					}
				}
			}
			else {
			// Check if it's a valid texture descriptor
			auto it2 = pipeline->textureDescriptorTable.find(parameter);
			if (it2 != pipeline->textureDescriptorTable.end()) {
				unsigned int texture_id = it2->second.texture_id;
				if (it2->second.paramType == PARAM_CUBEMAP) {
					auto handle = texture_manager->loadCubemap(dir + value);
					textures[texture_id].texture = texture_manager->getTexture(handle);
				}
				else {
					auto handle = texture_manager->loadTexture(dir + value);
					textures[texture_id].texture = texture_manager->getTexture(handle);
				}

				textures[texture_id].address = texture_id;
			}
			else {
				// Not a valid paramter
				GRIND_WARN("In {0}, invalid parameter {1}.", path, parameter);
			}
			}
		}
	}

	// Bind textures to pipeline in material
	TextureBinding *textureBinding = nullptr;
	if (textures.size() > 0) {
		TextureBindingCreateInfo ci;
		ci.layout = pipeline->tbl;
		ci.textures = textures.data();
		ci.textureCount = (uint32_t)textures.size();
		textureBinding = engine.getGraphicsWrapper()->CreateTextureBinding(ci);
	}

	UniformBuffer *ubo = nullptr;
	if (pipeline->parameterDescriptorTable.size() > 0) {
		UniformBufferCreateInfo uboci;
		uboci.isDynamic = false;
		uboci.size = pipeline->param_size;
		uboci.binding = pipeline->param_ubb;
		ubo = engine.getGraphicsWrapper()->CreateUniformBuffer(uboci);
		ubo->UpdateUniformBuffer(buffer);
	}

	// Make a new reference
	MaterialReference ref;
	ref.pipelineReference = pipeline_reference;
	ref.material = (uint32_t)pipeline->materials.size();

	// Place reference in material path
	pipeline->materials.emplace_back(ref, textureBinding);

	Material *mat = &pipeline->materials.back();
	mat->param_buffer_handler_ = ubo;
	mat->param_buffer_ = buffer;
	mat->path = path;

	material_map_[path] = ref;
	return ref;
}

MaterialReference MaterialManager::preloadMaterial(GeometryInfo geometry_info, std::string shaderName) {
	return MaterialReference();
}


Material *MaterialManager::getMaterial(MaterialReference ref) {
	auto pipeline_manager = engine.getGraphicsPipelineManager();
	auto pipeline = pipeline_manager->getPipeline(ref.pipelineReference);
	auto material = &pipeline->materials[ref.material];
	return material;
}

void MaterialManager::removeMaterial(MaterialReference ref) {
	auto pipeline_manager = engine.getGraphicsPipelineManager();
	auto pipeline = pipeline_manager->getPipeline(ref.pipelineReference);

	/*pipeline->materials.erase(pipeline->materials.begin() + ref.material);

	for (auto i = ref.material; i < pipeline->materials.size(); ++i) {
		pipeline->materials[i].reference.material = i;
		for (auto &mesh : pipeline->materials[i].m_meshes) {
			((MeshStatic *)mesh)->material_reference.material = i;
		}
	}

	if (pipeline->materials.size() == 0) {
		pipeline_manager->removePipeline(ref.pipelineReference);
	}*/
}

void MaterialManager::removeMeshFromMaterial(MaterialReference ref, Renderable*mesh) {
	Material *mat = getMaterial(ref);
	if (mat->m_meshes.size() > 1) {
		int index = 0; // Get Index from Reference by searching vector

		if (index + 1 != mat->m_meshes.size())
			std::swap(mat->m_meshes[index], mat->m_meshes.back());

		mat->m_meshes.pop_back();
	}
	else if (mat->m_meshes.size() == 1) {
		// Don't really need this but just in case.
		mat->m_meshes.pop_back();

		// Remove entire Material
		removeMaterial(ref);
	}
}

void MaterialManager::loadPreloaded()
{
}
