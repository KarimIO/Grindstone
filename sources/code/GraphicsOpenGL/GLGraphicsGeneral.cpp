#include "gl3w.h"
#include "OGLGraphicsWrapper.h"

#include "GLVertexArrayObject.h"
#include "GLVertexBufferObject.h"

GRAPHICS_EXPORT GraphicsWrapper* createGraphics() {
	return new GraphicsWrapper;
}

GRAPHICS_EXPORT void deletePointer(void * ptr) {
	free(ptr);
}

bool GraphicsWrapper::InitializeGraphics() {
	if (gl3wInit()) {
		printf("Failed to initialize GL3W. Returning...\n");
		return false;
	}

	/*if (!gl3wIsSupported(3, 3)) {
		printf("OpenGL %i.%i=< required for Grindstone Engine.\n", 3, 3);
		printf("Your Graphics Card only supports version %s. Quitting...\n\n", glGetString(GL_VERSION));
		return false;
	}*/

	printf("OpenGL %s initialized using GLSL %s.\n\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Depth Testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return true;
}

void GraphicsWrapper::SetResolution(int x, int y, uint32_t width, uint32_t height) {
	glViewport(x, y, width, height);
}

void GraphicsWrapper::DrawVertexArray(uint32_t numVertices) {
	glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
}

void GraphicsWrapper::DrawBaseVertex(ShapeType type, const void *baseIndex, uint32_t baseVertex, uint32_t numIndices) {
	int t = (type == SHAPE_PATCHES)?GL_PATCHES:type;
	glDrawElementsBaseVertex(
		t,
		numIndices,
		GL_UNSIGNED_INT,
		baseIndex,
		baseVertex);
	
}

unsigned char * GraphicsWrapper::ReadScreen(uint32_t width, uint32_t height) {
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	unsigned char *data = (unsigned char *)malloc(width * height * 3);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	glReadBuffer(GL_BACK);
	return data;
}

void GraphicsWrapper::Clear(unsigned int clearTarget) {
	if (clearTarget == CLEAR_COLOR)
		glClear(GL_COLOR_BUFFER_BIT);
	else if (clearTarget == CLEAR_DEPTH)
		glClear(GL_DEPTH_BUFFER_BIT);
	else
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GraphicsWrapper::SetDepth(int state) {
	if (state == 1) {
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
	}
	else if (state == 2) {
		glDepthFunc(GL_GEQUAL);
		glEnable(GL_DEPTH_TEST);
	}
	else
		glDisable(GL_DEPTH_TEST);
}

void GraphicsWrapper::SetTesselation(int verts) {
	GLint MaxPatchVertices = 0;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
	printf("Max supported patch vertices %d\n", MaxPatchVertices);
	glPatchParameteri(GL_PATCH_VERTICES, verts);
}

void GraphicsWrapper::SetCull(CullType state) {
	int cullState;
	switch (state) {
	case CULL_NONE:
		cullState = GL_FRONT_AND_BACK;
		break;
	case CULL_FRONT:
		cullState = GL_FRONT;
		break;
	default:
	case CULL_BACK:
		cullState = GL_BACK;
		break;
	}
	glCullFace(cullState);
}

void GraphicsWrapper::SetBlending(bool state) {
	if (state) {
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);
	}
	else {
		glDisable(GL_BLEND);
	}
}