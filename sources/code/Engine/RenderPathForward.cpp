#include "RenderPathForward.h"

void RenderPathForward::PrePass() {
	graphicsWrapper->Clear(CLEAR_ALL);
}

void RenderPathForward::GeometryPass() {
	geometryCache->Draw();
}

void RenderPathForward::PostPass() {
}

RenderPathForward::RenderPathForward(GraphicsWrapper * gw, SModel * gc) {
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

void RenderPathForward::Draw() {
	PrePass();
	GeometryPass();
	PostPass();
}
