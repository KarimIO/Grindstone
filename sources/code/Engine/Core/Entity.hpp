#ifndef _EBASE_H
#define _EBASE_H
#include "../Systems/CBase.h"
#include <stdint.h>

class Entity {
protected:
	uint32_t id;
public:
	Entity();
	unsigned int components[NUM_COMPONENTS];
};

#endif