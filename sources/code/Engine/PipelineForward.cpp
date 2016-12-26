#include "PipelineForward.h"

PipelineForward::PipelineForward(GraphicsWrapper * gw, SModel * gc) {
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

void PipelineForward::Draw() {
	graphicsWrapper->Clear();
	geometryCache->Draw();
}
