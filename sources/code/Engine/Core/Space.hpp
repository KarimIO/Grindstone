#ifndef _SPACE_H
#define _SPACE_H

#include <vector>
#include "Core/GameObject.hpp"
#include "rapidjson/document.h"

class Space {
public:
	Space(std::string name, rapidjson::Value &);

	void loadPrefab(std::string name, rapidjson::Value & val);
	SubSystem *getSubsystem(ComponentType type);
	GameObject &getObject(GameObjectHandle handle);
private:
	SubSystem *Space::addSystem(SubSystem * system);
	std::string name_;
	SubSystem *subsystems_[NUM_COMPONENTS];
	std::vector<GameObject> objects_;
};

#endif