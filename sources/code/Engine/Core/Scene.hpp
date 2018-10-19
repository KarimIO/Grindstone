#ifndef _LEVEL_LOADER_H
#define _LEVEL_LOADER_H

#include <vector>
#include <string>

class Space;

class Scene {
public:
	Scene(std::string path);
private:
	std::string path_;
	std::string name_;
	std::vector<Space *> spaces_;
};

#endif