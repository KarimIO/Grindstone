#ifndef _MODEL_MANAGER_H
#define _MODEL_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include "glm/glm.hpp"
#include <FormatCommon/Bounding.hpp>
#include <GraphicsCommon/IndexBuffer.hpp>
#include <GraphicsCommon/VertexArrayObject.hpp>
#include <GraphicsCommon/CommandBuffer.hpp>
#include "AssetManagers/AssetReferences.hpp"
#include "../AssetCommon/Renderable.hpp"

struct Vertex {
	glm::vec3 positions;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec2 texCoord;
};

enum {
	VERTEX_VB = 0,
	UV_VB,
	NORMAL_VB,
	TANGENT_VB,
	INDEX_VB,
	NUM_VBs
};

enum {
	VERTEX_VB_LOCATION = 0,
	UV_VB_LOCATION,
	NORMAL_VB_LOCATION,
	TANGENT_VB_LOCATION,
};

class Material;
struct ModelStatic;

struct ModelFormatHeader {
	uint32_t version;
	uint32_t num_meshes;
	uint64_t num_vertices;
	uint64_t num_indices;
	uint32_t num_materials;
	bool has_bones;
	bool large_index;
	BoundingType bounding_type;
};

class MeshStatic : public Renderable {
public:
	uint32_t num_indices = 0;
	uint32_t base_vertex = 0;
	uint32_t base_index = 0;
	MaterialReference material_reference;
	ModelReference model_reference;

	void shadowDraw() override;
	void draw() override;
};

struct MeshCreateInfo {
	uint32_t num_indices = 0;
	uint32_t base_vertex = 0;
	uint32_t base_index = 0;
	uint32_t material_index = UINT32_MAX;
};

#define BONES_PER_VERTEX 4

struct VertexWeights {
	uint16_t	bone_ids[BONES_PER_VERTEX];
	float		bone_weights[BONES_PER_VERTEX];
};

struct ModelStatic {
	bool loaded_ = false;
	ModelReference handle_;
	std::vector<ComponentHandle> references_;
	std::vector<MeshStatic> meshes;
	BoundingShape *bounding;
	Grindstone::GraphicsAPI::VertexBuffer *vertex_buffer;
	Grindstone::GraphicsAPI::VertexBuffer *shadow_vertex_buffer;
	Grindstone::GraphicsAPI::IndexBuffer *index_buffer;
	Grindstone::GraphicsAPI::VertexArrayObject *vertex_array_object;
	Grindstone::GraphicsAPI::VertexArrayObject *shadow_vertex_array_object;
	Grindstone::GraphicsAPI::CommandBuffer *command_buffer;
	std::string path_;

	ModelStatic();
	ModelStatic(ModelReference handle, std::string path, ComponentHandle ref);
};

class ModelManager {
public:
	ModelManager(Grindstone::GraphicsAPI::UniformBufferBinding *ubb);
	void prepareGraphics();
	ModelReference preloadModel(ComponentHandle, std::string);
	ModelReference loadModel(ComponentHandle, std::string);
	void loadPreloaded();
	ModelStatic &getModel(ModelReference);
	Grindstone::GraphicsAPI::UniformBuffer *getModelUbo();
	void removeModelInstance(ModelReference, ComponentHandle);
	void destroyGraphics();
	void reloadGraphics();
private:
	std::map<std::string, ModelReference> models_map_;
	std::vector<ModelStatic> models_;
	std::vector<ModelReference> unloaded_;
	GeometryInfo geometry_info_;
	Grindstone::GraphicsAPI::UniformBufferBinding *model_ubb_;
	Grindstone::GraphicsAPI::UniformBuffer *model_ubo_;
	std::vector<Grindstone::GraphicsAPI::UniformBufferBinding *> ubbs_;

	MaterialReference empty_material;

	void destroyModel(ModelReference ref);

	bool loadModel(ModelStatic &model);
	Grindstone::GraphicsAPI::VertexBufferLayout vertex_layout_;
};

#endif