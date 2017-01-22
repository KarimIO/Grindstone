#ifndef _C_BASE_H
#define _C_BASE_H

enum {
	COMPONENT_BASE = 0,
	COMPONENT_MODEL,
	COMPONENT_RENDER,
	COMPONENT_LIGHT_POINT,
	COMPONENT_LIGHT_SPOT,
	COMPONENT_LIGHT_DIRECTIONAL,
	NUM_COMPONENTS
};

class CBase {
public:
	unsigned char componentType;
	unsigned int entityID;
};

#endif