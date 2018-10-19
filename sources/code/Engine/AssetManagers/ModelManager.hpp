#ifndef _MODEL_MANAGER_H
#define _MODEL_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include "glm/glm.hpp"
#include "../FormatCommon/Bounding.hpp"
#include <IndexBuffer.hpp>
#include <VertexArrayObject.hpp>
#include <CommandBuffer.hpp>
#include "AssetManagers/AssetReferences.hpp"

struct Vertex {
	glm::vec3 positions;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec2 texCoord;
};

typedef uint32_t ModelReference;

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
	uint16_t num_bones;
	bool large_index;
	BoundingType bounding_type;
};

class MeshStatic {
public:
	uint32_t num_indices = 0;
	uint32_t base_vertex = 0;
	uint32_t base_index = 0;
	MaterialReference material_reference;
	ModelReference model_reference;

	virtual void shadowDraw();
	virtual void draw();
};

struct MeshCreateInfo {
	uint32_t num_indices = 0;
	uint32_t base_vertex = 0;
	uint32_t base_index = 0;
	uint32_t material_index = UINT32_MAX;
};

struct ModelStatic {
	ModelReference id;
	std::vector<uint32_t> references_;
	std::vector<MeshStatic> meshes;
	BoundingShape *bounding;
	VertexBuffer *vertex_buffer;
	VertexBuffer *shadow_vertex_buffer;
	IndexBuffer *index_buffer;
	VertexArrayObject *vertex_array_object;
	VertexArrayObject *shadow_vertex_array_object;
	CommandBuffer *command_buffer;
	std::string path_;

	ModelStatic() {}
	ModelStatic(std::string path, std::vector<uint32_t> refs) : path_(path), references_(refs) {};
};

class ModelManager {
public:
	ModelManager();
	void preloadModel(std::string);
	void loadModel(std::string);
	void loadPreloaded();
	ModelStatic &getModel(ModelReference);
private:
	std::map<std::string, ModelReference> models_map_;
	std::vector<ModelStatic> models_;
	std::vector<ModelReference> unloaded_;
	GeometryInfo geometry_info_;

	void loadModel(ModelStatic &model);
};

#endif