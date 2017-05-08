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

void SwitchSlashes(std::string &path);

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
	std::vector<unsigned int> references;
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
	std::vector<CRender> renderComponents;
	std::vector<unsigned int> unloadedModelIDs;
	void LoadModel3DFile(const char *szPath, CModel *model);
	void InitMesh(const aiMesh *paiMesh,
		std::vector<glm::vec3>& vertices,
		std::vector<glm::vec3>& normals,
		std::vector<glm::vec3>& tangents,
		std::vector<glm::vec2>& uvs,
		std::vector<unsigned int>& indices);
public:
	void AddComponent(unsigned int entID, unsigned int &target);

	void LoadModel3D(const char *szPath, size_t entityID);
	void PreloadModel3D(const char * szPath, size_t renderID);

	void LoadPreloaded();

	void InitMaterials(const aiScene* scene, std::string Dir, std::vector<Material *> &model);

	void Draw(glm::mat4 projection, glm::mat4 view);
	void DrawModel3D(glm::mat4 projection, glm::mat4 view, CModel *);

	void ShadowDraw(glm::mat4 projection, glm::mat4 view);
	void ShadowDrawModel3D(glm::mat4 projection, glm::mat4 view, CModel *);

	void Shutdown();
};

#endif