#ifndef _SGEOMETRY_H
#define _SGEOMETRY_H

#include "../GraphicsCommon/Shader.h"
//#include "../GraphicsCommon/Texture.h"
#include "../GraphicsCommon/VertexArrayObject.h"

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
};

class CRender {
private:
public:
	std::vector<Material *> materials;
};

class SModel;

class CModel {
	friend class SModel;
private:
	std::vector<CRender *> references;
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

	unsigned int vao;
	unsigned int m_Buffers[NUM_VBs];

	// Don't attach to CRender
	CModel *PrepareModel3D(const char *szPath);
	CModel *LoadModel3D(const char *szPath);

	// Attach to CRender
	CModel *PrepareModel3D(const char *szPath, CRender *);
	CModel *LoadModel3D(const char *szPath, CRender *);
	void LoadPreparedModel3Ds();

	void Draw();
	void DrawModel3D(CModel *);
};

#endif