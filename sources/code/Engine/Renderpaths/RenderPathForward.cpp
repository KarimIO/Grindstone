#include "RenderPathForward.h"

void RenderPathForward::PrePass() {
	graphicsWrapper->Clear(CLEAR_ALL);
}

void RenderPathForward::GeometryPass(glm::mat4 projection, glm::mat4 view) {
	geometryCache->Draw(projection, view);
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

void RenderPathForward::Draw(glm::mat4 projection, glm::mat4 view, glm::vec3 eyePos, bool usePost) {
	PrePass();
	GeometryPass(projection, view);
	PostPass();
}

Framebuffer * RenderPathForward::GetGBuffer()
{
	return nullptr;
}

Framebuffer * RenderPathForward::GetFramebuffer()
{
	return nullptr;
}
