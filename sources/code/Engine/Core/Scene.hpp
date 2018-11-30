#ifndef _LEVEL_LOADER_H
#define _LEVEL_LOADER_H

#include <vector>
#include <string>
#include "rapidjson/document.h"

class Space;

class Scene {
public:
	Scene(std::string path);
	std::string getName();
	std::string getPath();
	std::vector<Space *> spaces_;
private:
	void loadLevel(std::string path);
	std::string path_;
	std::string name_;
};

#endif