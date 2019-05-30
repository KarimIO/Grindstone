#ifndef _RENDER_TERRAIN_SYSTEM_H
#define _TERRAIN_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "BaseSystem.hpp"
#include "../AssetCommon/Renderable.hpp"

class VertexBuffer;
class IndexBuffer;
class VertexArrayObject;
class Texture;

class TerrainDrawable : public Renderable {
public:
	ComponentHandle component_handle_;
	VertexBuffer *vertex_buffer;
	IndexBuffer *index_buffer;
	VertexArrayObject *vertex_array_object;
	unsigned int num_indices_;
	UniformBufferBinding *model_ubb_;
	UniformBuffer *model_ubo_;

	void shadowDraw() override;
	void draw() override;
};

struct RenderTerrainComponent : public Component {
	RenderTerrainComponent(GameObjectHandle object_handle, ComponentHandle handle);
	std::string path_;
	std::string material_path_;
	MaterialReference material_;
	Texture *heightmap_;
	char *heightmap_data_;
	unsigned int heightmap_size_;
	void generateMesh();
	TerrainDrawable *terrain_drawable_;
};

class RenderTerrainSystem : public System {
public:
	RenderTerrainSystem(UniformBufferBinding *ubb);

	void update(double dt);
	UniformBuffer *getModelUbo();
	GeometryInfo geometry_info_;
private:
	UniformBufferBinding *model_ubb_;
	UniformBuffer *model_ubo_;
	std::vector<UniformBufferBinding *> ubbs_;
};

class RenderTerrainSubSystem : public SubSystem {
	friend RenderTerrainSystem;
public:
	RenderTerrainSubSystem(RenderTerrainSystem *system, Space *space);
	virtual ComponentHandle addComponent(GameObjectHandle object_handle) override;
	virtual ComponentHandle addComponent(GameObjectHandle object_handle, rapidjson::Value &params) override;
	RenderTerrainComponent &getComponent(ComponentHandle handle);
	size_t getNumComponents();
	virtual void writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) override;
	virtual void removeComponent(ComponentHandle handle);
	virtual ~RenderTerrainSubSystem();
private:
	std::vector<RenderTerrainComponent> components_;
	RenderTerrainSystem *system_;
};

/*class CTerrain : public Mesh {
	friend class SGeometryTerrain;
public:
	CTerrain(std::string path, std::vector<uint32_t> refs) : heightmap_dir_(path), references(refs) {};

	virtual std::string getHeightmap();
	virtual void setHeightmap(std::string dir);

	virtual void ShadowDraw();
	virtual void Draw();
	virtual void DrawDeferred(CommandBuffer *);
private:
	std::vector<uint32_t> references;
	VertexBuffer *vertexBuffer;
	IndexBuffer *indexBuffer;
	VertexArrayObject *vertexArrayObject;
	CommandBuffer *commandBuffer;
	Texture *heightmap_texture_;
	TextureBinding *heightmap_texture_binding_;
	std::string heightmap_dir_;
	unsigned int num_indices_;

	Material *material_;
};

class SGeometryTerrain : public SSubGeometry {
public:
	SGeometryTerrain(MaterialManager * material_system, GraphicsWrapper * graphics_wrapper, std::vector<UniformBufferBinding*> ubbs);
	void LoadModel(CTerrain *model);
	virtual void LoadGeometry(unsigned int render_id, std::string path);
	virtual void LoadPreloaded();
	virtual void Cull(CCamera *cam);
private:
	MaterialManager *material_system_;
	GraphicsWrapper *graphics_wrapper_;
	GeometryInfo geometry_info_;

	std::vector<CTerrain> models;
	std::vector<unsigned int> unloadedModelIDs;
};*/

#endif