#ifndef _RENDERPATH_H
#define _RENDERPATH_H

#include <GraphicsWrapper.h>
#include "SGeometry.h"

class RenderPath {
public:
	virtual void Draw() = 0;
};

#endif