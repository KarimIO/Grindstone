#include "gl3w.h"
#include "OGLGraphicsWrapper.h"

#include "GLVertexArrayObject.h"
#include "GLVertexBufferObject.h"

GRAPHICS_EXPORT GraphicsWrapper* createGraphics() {
	std::cout << "Creating the graphics (dll-side)\n";
	return new GraphicsWrapper;
}

bool GraphicsWrapper::InitializeGraphics()
{
	if (gl3wInit()) {
		printf("Failed to initialize GL3W. Returning...\n");
		return false;
	}

	if (!gl3wIsSupported(3, 3)) {
		printf("OpenGL %i.%i=< required for Grind Engine. Returning...\n", 3, 3);
		return false;
	}

	printf("Windowing and GL3W initialized.\nOpenGL %s, GLSL %s.\n\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

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
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void GraphicsWrapper::DrawBaseVertex(const void *baseIndex, uint32_t baseVertex, uint32_t numIndices) {
	glDrawElementsBaseVertex(
		GL_TRIANGLES,
		numIndices,
		GL_UNSIGNED_INT,
		baseIndex,
		baseVertex);
	
}

void GraphicsWrapper::Clear(unsigned int clearTarget) {
	if (clearTarget == CLEAR_COLOR)
		glClear(GL_COLOR_BUFFER_BIT);
	else if (clearTarget == CLEAR_DEPTH)
		glClear(GL_DEPTH_BUFFER_BIT);
	else
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
