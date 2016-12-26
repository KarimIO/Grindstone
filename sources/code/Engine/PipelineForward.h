#ifndef _PIPELINE_FORWARD_H
#define _PIPELINE_FORWARD_H

#include "Pipeline.h"

class PipelineForward : public Pipeline {
	float *quadVertices;
	GraphicsWrapper *graphicsWrapper;
	SModel *geometryCache;
public:
	PipelineForward(GraphicsWrapper *gw, SModel *gc);
	virtual void Draw();
};

#endif