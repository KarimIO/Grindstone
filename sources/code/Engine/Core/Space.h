#ifndef _SPACE_H
#define _SPACE_H

#include <vector>
#include "System.h"
#include "../Systems/CBase.h"

class Space {
public:
	void AddComponent(ComponentType type);
	std::vector<SpaceComponentList *> components;
};

#endif