#ifndef _SPACE_H
#define _SPACE_H

#include <vector>
#include "System.hpp"
#include "../Systems/CBase.hpp"

class Space {
public:
	void AddComponent(ComponentType type);
	std::vector<SpaceComponentList *> components;
};

#endif