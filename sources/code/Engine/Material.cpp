#define _CRT_SECURE_NO_WARNINGS

//C Standard Library
#include <cstdlib>
#include <cstdio>

//C++ Standard Library
#include <iostream>
#include <vector>
#include <string>

#include <cstring>

typedef unsigned int uint;

class Texture {
public:
	std::string path;
};

struct Material {
	std::string				name;
	std::vector<Texture *>	textures;
	std::vector<uint>		uints;
	std::vector<int>		ints;
};

class MaterialFile {
public:
	MaterialFile(const char *);
	bool read(const char *);
	bool parseBuffer(char *, const unsigned int);
	char *getCStr();
	int   getInt();
	int   getInt2();
	int   getInt3();
	int   getInt4();
	unsigned int   getUInt();
	unsigned int   getUInt2();
	unsigned int   getUInt3();
	unsigned int   getUInt4();
	float getFloat();
	float getFloat2();
	float getFloat3();
	float getFloat4();
	std::vector<Material *> materials;
};

MaterialFile::MaterialFile(const char *path) {
	read(path);
}

char *MaterialFile::getCStr() {
	return strtok(NULL, " \n\t");
}

int MaterialFile::getInt() {
	return atoi(getCStr());
}

int MaterialFile::getInt2() {
	return getInt() + getInt();
}

int MaterialFile::getInt3() {
	return getInt() + getInt() + getInt();
}

int MaterialFile::getInt4() {
	return getInt() + getInt() + getInt() + getInt();
}

unsigned int MaterialFile::getUInt() {
	return atoi(getCStr());
}

unsigned int MaterialFile::getUInt2() {
	return getUInt() + getUInt();
}

unsigned int MaterialFile::getUInt3() {
	return getUInt() + getUInt() + getUInt();
}

unsigned int MaterialFile::getUInt4() {
	return getUInt() + getUInt() + getUInt() + getUInt();
}

float MaterialFile::getFloat() {
	return atof(getCStr());
}

float MaterialFile::getFloat2() {
	return getFloat() + getFloat();
}

float MaterialFile::getFloat3() {
	return getFloat() + getFloat() + getFloat();
}

float MaterialFile::getFloat4() {
	return getFloat() + getFloat() + getFloat() + getFloat();
}

bool MaterialFile::parseBuffer(char *buffer, unsigned int size) {
	char *ptr = strtok(buffer, " \n\t");
	int iterator = -1;
	while (ptr != NULL) {
		if (strcmp(ptr, "numMats") == 0) {
			materials.reserve(getInt());
		}
		else if (strcmp(ptr, "mat") == 0) {
			materials.push_back(new Material());
			materials[++iterator]->name = getCStr();
		}
		else if (strcmp(ptr, "numTex") == 0) {
			materials[iterator]->textures.reserve(getInt());
		}
		else if (strcmp(ptr, "tex") == 0) {
			Texture *tex = new Texture();
			tex->path = getCStr();
			materials[iterator]->textures.push_back(tex);
		}

		// Get the next token
		ptr = getCStr();
	}

	return true;
}

bool MaterialFile::read(const char *path) {
	FILE *pFile = 0;
#ifdef _MSC_VER
	pFile = fopen(path, "r");
#else
	pFile = fopen(path, "r");
#endif

	if (pFile == NULL) {
		printf("File %s failed to load\n", path);
		return false;
	}

	// Get the size of the file.
	fseek(pFile, 0, SEEK_END);
	unsigned int fileSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	// Allocate space.
	char *buffer = (char *)malloc(fileSize + 1);

	fileSize = fread(buffer, 1, fileSize, pFile);
	buffer[fileSize] = 0;

	parseBuffer(buffer, fileSize);

	for (size_t i = 0; i < materials.size(); i++) {
		Material *targetMat = materials[i];
		for (size_t j = 0; j < targetMat->textures.size(); j++) {
			Texture *targetTex = targetMat->textures[j];
			std::cout << targetTex->path << "\n";
		}
	}

	free(buffer);

	fclose(pFile);
	
	return false;
}