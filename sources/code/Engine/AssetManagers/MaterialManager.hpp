#ifndef _MATERIAL_MANGER_H
#define _MATERIAL_MANGER_H

#include <map>
#include <vector>
#include "AssetReferences.hpp"
#include "GraphicsPipelineManager.hpp"
#include "../AssetCommon/Renderable.hpp"

const MaterialReference invalid_material = {{(uint8_t)-1, (uint8_t)-1, TYPE_MISSING}, (uint16_t)-1};

class MeshStatic;

class MaterialManager;

class Material {
	friend MaterialManager;
public:
	TextureBinding *m_textureBinding;
	UniformBuffer *param_buffer_handler_;
	char *param_buffer_;
	std::vector<Renderable *> m_meshes;
	Material() {
		m_textureBinding = 0;
		m_meshes.clear();
	}
	Material(MaterialReference reference, TextureBinding *textureBinding);
	void incrementDrawCount();
	const uint32_t getDrawCount();
	std::string path;
private:
	MaterialReference reference;
	uint32_t draw_count;
};

class MaterialManager {
public:
	MaterialReference loadMaterial(GeometryInfo geometry_info, std::string shaderName);
	MaterialReference preloadMaterial(GeometryInfo geometry_info, std::string shaderName);

	Material *getMaterial(MaterialReference);

	void removeMaterial(MaterialReference);
	void removeMeshFromMaterial(MaterialReference, Renderable*);

	void loadPreloaded();

	void reloadAll();
	void reloadMaterial(MaterialReference handle);
	
	void cleanup();
	~MaterialManager();
	std::map<std::string, MaterialReference> material_map_;
private:
};

#endif