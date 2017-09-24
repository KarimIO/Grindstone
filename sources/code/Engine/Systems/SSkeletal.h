#if 0
#ifndef _S_SKELETAL_H
#define _S_SKELETAL_H

#include <vector>
#include "SGeometry.h"
#include <glm/gtc/quaternion.hpp>

struct JointTransform {
	glm::vec3 position;
	glm::quat rotation;
};

struct AnimKeyframe {
	std::vector<JointTransform> transforms;
	float time;
};

struct Animation {
	std::vector<AnimKeyframe> keyframes;
	std::string animName;
};

class CRenderSkeletal {
private:
public:
	size_t entityID;
	std::vector<Material *> materials;
	unsigned int animationID;
	float animationTime;
};

class CSkeletal {
	friend class SSkeletal;
private:
	std::vector<unsigned int> references;
	std::vector<Material *> materials;
	std::vector<Mesh> meshes;
	std::vector<Animation> animations;
	VertexArrayObject *vao;
	std::string name;
public:
	std::string getName();
};

class SSkeletal : public SModel {
	friend class CSkeletal;
private:
	std::vector<CSkeletal> models;
	std::vector<CRenderSkeletal> references;
	void LoadFile(const char *szPath, CSkeletal *model);
	void LoadModel3DFile(const char *szPath, CModel *model);
	void InitMesh(const aiMesh *paiMesh,
		std::vector<glm::vec3>& vertices,
		std::vector<glm::vec3>& normals,
		std::vector<glm::vec3>& tangents,
		std::vector<glm::vec2>& uvs,
		std::vector<unsigned int>& indices);

	void InitBones(const aiMesh *paiMesh,
		std::vector<glm::tvec4<unsigned char, glm::highp>>& boneID,
		std::vector<glm::vec4>& boneWeights);

	void InitMaterials(const aiScene* scene, std::string Dir, std::vector<Material *> &materials);

public:
	void AddComponent(unsigned int entID, unsigned int &target);

	void LoadModel3D(const char *szPath, size_t entityID);
	void PreloadModel3D(const char * szPath, size_t renderID);

	void LoadPreloaded();

	void CalculateAnimations(unsigned int modelID, unsigned int refID, std::vector<JointTransform> &joints);

	
	void Draw(glm::mat4 projection, glm::mat4 view);
	void DrawModel3D(glm::mat4 projection, glm::mat4 view, CModel *);

	void Shutdown();
};

#endif
#endif