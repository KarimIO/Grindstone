#ifndef _TERRAIN_H
#define _TERRAIN_H

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <Shader.h>
#include <VertexArrayObject.h>
#include <Texture.h>

class CTerrain {
public:
	Texture *texture;
	VertexArrayObject *vao;
	unsigned int numPatches;
	unsigned int numIndices;
	float height;
};

class STerrain {
private:
	ShaderProgram *terrainShader;
	Texture *grassAlbedo;
	Texture *grassGeometry;

	Texture *rockAlbedo;
	Texture *rockGeometry;
public:
	std::vector<CTerrain> components;
	virtual void Initialize();
	virtual void LoadTerrain(std::string path, float width, float length, float height, unsigned int patches);
	virtual void Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos);
};

#endif