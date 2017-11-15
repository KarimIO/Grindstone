#ifndef _EBASE_H
#define _EBASE_H
#include "../Systems/CBase.hpp"
#include <stdint.h>

class Entity {
protected:
	uint32_t id_;
public:
	Entity();
	unsigned int components_[NUM_COMPONENTS];
	~Entity();
};

#endif