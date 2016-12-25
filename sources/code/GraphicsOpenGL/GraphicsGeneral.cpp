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
	std::cout << "poop \n";

	//Initializing GL3W
	int vermaj;
	int vermin;
	glGetIntegerv(GL_MAJOR_VERSION, &vermaj);
	glGetIntegerv(GL_MINOR_VERSION, &vermin);

	std::cout << vermaj << " - " << vermin << "\n";
	std::cout << "poop \n";

	int gl3wres = gl3wInit();
	if (gl3wres) {
		printf("Failed to initialize GL3W. Returning %i...\n", gl3wres);
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return true;
}

void GraphicsWrapper::DrawBaseVertex(const void *baseIndex, uint32_t baseVertex, uint32_t numIndices) {
	glDrawElementsBaseVertex(
		GL_TRIANGLES,
		numIndices,
		GL_UNSIGNED_INT,
		baseIndex,
		baseVertex);
}

void GraphicsWrapper::DrawArrays(VertexArrayObject *vao, int start, unsigned int length) {
	vao->Bind();
	glDrawArrays(GL_TRIANGLES, 0, 3);
	vao->Unbind();
}

void GraphicsWrapper::Clear() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
