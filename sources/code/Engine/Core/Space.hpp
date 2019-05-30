#ifndef _SPACE_H
#define _SPACE_H

#include <vector>
#include "Core/GameObject.hpp"
#include "rapidjson/document.h"

class Space {
public:
	Space(std::string name, rapidjson::Value &);
	Space(const Space &s);
	void clear();
	~Space();

	void loadPrefab(std::string name, rapidjson::Value & val);
	SubSystem *getSubsystem(ComponentType type);
	GameObject &getObject(GameObjectHandle handle);
	std::string getName();
	std::vector<GameObject> objects_;
private:
	SubSystem *addSystem(SubSystem * system);
	std::string name_;
	SubSystem *subsystems_[NUM_COMPONENTS];
};

#endif