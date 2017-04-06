#ifndef _MATERIALS_H
#define _MATERIALS_H

#include "SShader.h"

class Material {
public:
	ShaderProgram *shader;
	std::vector<Texture *> tex;
	char *uniformData;
};

class MaterialSystem {
	std::map<std::string, Material *> materials;
	void NewMaterial(std::string name, ParameterDescriptor);
	void SetParameter(unsigned index, void *dataPtr);
public:
	void ParseMaterialFile(const char *path);
	void ParseMaterialManifest(const char *path);
};

#endif