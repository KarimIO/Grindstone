#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include <vector>
#include "../Systems/BaseSystem.hpp"


class GameObject {
public:
	bool operator== (GameObject &other);
	GameObjectHandle getID();
	~GameObject();
private:
	GameObjectHandle id_;
	std::vector<ComponentHandle> handler_;
};

#endif