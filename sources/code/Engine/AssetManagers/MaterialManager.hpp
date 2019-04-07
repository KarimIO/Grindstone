#ifndef _MATERIAL_MANGER_H
#define _MATERIAL_MANGER_H

#include <map>
#include <vector>
#include "AssetReferences.hpp"
#include "GraphicsPipelineManager.hpp"
#include "../AssetCommon/Renderable.hpp"

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
	
	void cleanup();
	~MaterialManager();
private:
	std::map<std::string, MaterialReference> material_map_;

	MaterialReference empty_material_reference_;
};

#endif