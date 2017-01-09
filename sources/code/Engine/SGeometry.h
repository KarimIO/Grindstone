#ifndef _SGEOMETRY_H
#define _SGEOMETRY_H

#include "../GraphicsCommon/Shader.h"
#include "../GraphicsCommon/Texture.h"
#include "../GraphicsCommon/VertexArrayObject.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>

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

struct Mesh {
	unsigned int NumIndices = 0;
	unsigned int BaseVertex = 0;
	unsigned int BaseIndex = 0;
	unsigned char MaterialIndex = 255;
};

class Material {
public:
	ShaderProgram *shader;
	std::vector<Texture *> tex;
};

class CRender {
private:
public:
	size_t entityID;
	std::vector<Material *> materials;
};

class SModel;

class CModel {
	friend class SModel;
private:
	std::vector<CRender> references;
	std::vector<Material *> materials;
	std::vector<Mesh> meshes;
	VertexArrayObject *vao;
	std::string name;
public:
	std::string getName();
};

class SModel {
public:
	std::vector<CModel> models;
	std::vector<CModel*> unloadedModels;
	void LoadModel3DFile(const char *szPath, CModel *model);
public:

	void LoadModel3D(const char *szPath, size_t entityID, size_t &modelID, size_t &renderID);

	void InitMaterials(const aiScene* scene, std::string Dir, CModel *model);

	void Draw(glm::mat4 projection, glm::mat4 view);
	void DrawModel3D(glm::mat4 projection, glm::mat4 view, CModel *);
};

#endif