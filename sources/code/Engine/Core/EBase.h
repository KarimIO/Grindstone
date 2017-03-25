#ifndef _EBASE_H
#define _EBASE_H
#include "../Systems/CBase.h"

class EBase {
protected:
	unsigned id;
public:
	EBase();
	unsigned int components[NUM_COMPONENTS];
};

#endif