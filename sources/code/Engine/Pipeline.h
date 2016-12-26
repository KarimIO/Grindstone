#ifndef _PIPELINE_H
#define _PIPELINE_H

#include <GraphicsWrapper.h>
#include "SGeometry.h"

class Pipeline {
public:
	virtual void Draw() = 0;
};

#endif