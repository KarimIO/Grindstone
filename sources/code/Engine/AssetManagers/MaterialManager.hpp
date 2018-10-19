#ifndef _MATERIAL_MANGER_H
#define _MATERIAL_MANGER_H

#include <map>
#include <vector>
#include "AssetReferences.hpp"
#include "GraphicsPipelineManager.hpp"

class MeshStatic;

class MaterialManager;

class Material {
	friend MaterialManager;
public:
	TextureBinding *m_textureBinding;
	std::vector<MeshStatic *> m_meshes;
	Material() {
		m_textureBinding = 0;
		m_meshes.clear();
	}
	Material(MaterialReference reference, TextureBinding *textureBinding);
	void incrementDrawCount();
	const uint32_t getDrawCount();
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
	void removeMeshFromMaterial(MaterialReference, MeshStatic*);

	void loadPreloaded();
	
	void cleanup();
	~MaterialManager();
private:
	std::map<std::string, MaterialReference> material_map_;

	MaterialReference empty_material_reference_;
};

#endif