#ifndef _S_GEOMETRY_STATIC_H
#define _S_GEOMETRY_STATIC_H

#include "SGeometry.hpp"
#include "../FormatCommon/Bounding.hpp"

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

class CModelStatic;

class MeshStatic : public Mesh {
public:
	uint32_t NumIndices = 0;
	uint32_t BaseVertex = 0;
	uint32_t BaseIndex = 0;
	Material *material;
	CModelStatic *model = nullptr;

	virtual void Draw();
	virtual void DrawDeferred(CommandBuffer *);
};

struct MeshCreateInfo {
	uint32_t NumIndices = 0;
	uint32_t BaseVertex = 0;
	uint32_t BaseIndex = 0;
	uint32_t MaterialIndex = UINT32_MAX;
};

class CModelStatic : public Geometry {
	friend class SGeometryStatic;
public:
	BoundingShape *bounding;
	bool useLargeBuffer = true;
	std::vector<uint32_t> references;
	std::vector<MeshStatic> meshes;
	VertexBuffer *vertexBuffer;
	IndexBuffer *indexBuffer;
	VertexArrayObject *vertexArrayObject;
	CommandBuffer *commandBuffer;
	std::string name;

	CModelStatic(std::string path, std::vector<uint32_t> refs) : name(path), references(refs) {};

	virtual std::string getName();
	virtual void setName(std::string dir);
};

class SGeometryStatic : public SSubGeometry {
public:
	SGeometryStatic(MaterialManager *material_system, GraphicsWrapper *graphics_wrapper, VertexBindingDescription vbd, std::vector<VertexAttributeDescription> vads);

	virtual void LoadGeometry(uint32_t render_id, std::string path);
	virtual void LoadPreloaded();

	virtual void Cull();

	~SGeometryStatic();
private:
	GraphicsWrapper *graphics_wrapper_;
	MaterialManager *material_system_;
	VertexBindingDescription vbd_;
	std::vector<VertexAttributeDescription> vads_;
	std::vector<CommandBuffer *> GetCommandBuffers();

	std::vector<CModelStatic> models;
	std::vector<unsigned int> unloadedModelIDs;
	void LoadModel(CModelStatic *model);
};

#endif