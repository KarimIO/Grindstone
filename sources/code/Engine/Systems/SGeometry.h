#ifndef _SGEOMETRY_H
#define _SGEOMETRY_H

#include "../GraphicsCommon/GraphicsWrapper.h"
#include "../GraphicsCommon/Texture.h"
#include "../GraphicsCommon/CommandBuffer.h"
#include "../GraphicsCommon/VertexArrayObject.h"

#include <glm/glm.hpp>
#include <vector>

#include "SShader.h"

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
	//BONE_VB,
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

class Material;

class CRender {
private:
public:
	size_t entityID;
	std::vector<Material *> materials;
};

class CModel {
	friend class SModel;
public:
	bool useLargeBuffer = true;
	std::vector<unsigned int> references;
	std::vector<Mesh> meshes;
	VertexBuffer *vertexBuffer;
	IndexBuffer *indexBuffer;
	VertexArrayObject *vertexArrayObject;
	CommandBuffer *commandBuffer;
	std::string name;

	std::string getName();
};

class SModel {
public:
	GraphicsWrapper *graphicsWrapper;
	MaterialManager *materialManager;

	VertexBindingDescription vbd;
	std::vector<VertexAttributeDescription> vads;

	std::vector<CModel> models;
	std::vector<CRender> renderComponents;
	std::vector<unsigned int> unloadedModelIDs;
	bool LoadModel3DFile(const char *szPath, CModel *model);
public:
	void Initialize(GraphicsWrapper *graphicsWrapper, VertexBindingDescription vbd, std::vector<VertexAttributeDescription> vads, MaterialManager *materialManager);
	void AddComponent(unsigned int entID, unsigned int &target);

	void LoadModel3D(const char *szPath, size_t entityID);
	void PreloadModel3D(const char * szPath, size_t renderID);

	void LoadPreloaded();

	std::vector<CommandBuffer *> GetCommandBuffers();

	void Shutdown();
};

#endif