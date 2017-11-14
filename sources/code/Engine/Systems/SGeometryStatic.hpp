#ifndef _S_GEOMETRY_STATIC_H
#define _S_GEOMETRY_STATIC_H

#include "SGeometry.h"

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

struct ModelFormatHeader {
	uint32_t version;
	uint32_t numMeshes;
	uint64_t numVertices;
	uint64_t numIndices;
	uint32_t numMaterials;
	uint8_t numBones;
	bool largeIndex;
};

class MeshStatic : public Mesh {
public:
	uint32_t NumIndices = 0;
	uint32_t BaseVertex = 0;
	uint32_t BaseIndex = 0;
	CModelStatic *model = nullptr;

	virtual void Draw();
	virtual void DrawDeferred(CommandBuffer *);
};

class CModelStatic {
	friend class SGeometryStatic;
public:
	bool useLargeBuffer = true;
	std::vector<unsigned int> references;
	std::vector<MeshStatic> meshes;
	VertexBuffer *vertexBuffer;
	IndexBuffer *indexBuffer;
	VertexArrayObject *vertexArrayObject;
	CommandBuffer *commandBuffer;
	std::string name;

	std::string getName();
};

class SGeometryStatic : public SSubGeometry {
public:
	SGeometryStatic(GraphicsWrapper *graphics_wrapper_);

	virtual CGeometry *PreloadComponent(std::string path);
	virtual CGeometry *LoadComponent(std::string path);
	virtual void LoadPreloaded();

	void AddComponent(unsigned int entID, unsigned int &target);

	void LoadModel3D(const char *szPath, size_t entityID);
	void PreloadModel3D(const char * szPath, size_t renderID);

	std::vector<CommandBuffer *> GetCommandBuffers();

	~SGeometryStatic();
private:
	GraphicsWrapper *graphics_wrapper_;
	MaterialManager *material_system_;
	VertexBindingDescription vbd_;
	std::vector<VertexAttributeDescription> vads_;

	std::vector<CModelStatic> models;
	std::vector<CRender> renderComponents;
	std::vector<unsigned int> unloadedModelIDs;
	void LoadModel(CModelStatic *model);
};

#endif