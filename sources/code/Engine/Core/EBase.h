#ifndef _EBASE_H
#define _EBASE_H
#include "../Systems/CBase.h"
#include <stdint.h>

class EBase {
protected:
	uint32_t id;
public:
	EBase();
	unsigned int components[NUM_COMPONENTS];
};

#endif