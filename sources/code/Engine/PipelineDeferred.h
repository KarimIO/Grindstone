#ifndef _PIPELINE_DEFERRED_H
#define _PIPELINE_DEFERRED_H

#include "Pipeline.h"

class PipelineDeferred {
	float *quadVertices;
	GraphicsWrapper *graphicsWrapper;
	SModel *geometryCache;
public:
	PipelineDeferred(GraphicsWrapper *gw, SModel *gc);
	virtual void Draw();
};

#endif