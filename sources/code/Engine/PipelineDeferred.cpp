#include "PipelineDeferred.h"

PipelineDeferred::PipelineDeferred(GraphicsWrapper * gw, SModel * gc) {
	float tempVerts[8] = {
		-1,-1,
		-1, 1,
		1, 1,
		1,-1
	};
	quadVertices = tempVerts;

	graphicsWrapper = gw;
	geometryCache = gc;
}

void PipelineDeferred::Draw() {
	graphicsWrapper->Clear();
	geometryCache->Draw();
}
