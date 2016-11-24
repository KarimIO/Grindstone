#include "gl3w.h"
#include "OGLGraphicsWrapper.h"

#include "GLVertexArrayObject.h"
#include "GLVertexBufferObject.h"

GRAPHICS_EXPORT GraphicsWrapper* createGraphics() {
	std::cout << "Creating the graphics (dll-side)\n";
	return new GraphicsWrapper;
}

GRAPHICS_EXPORT VertexArrayObject* createVAO() {
	std::cout << "Creating the VAO\n";
	return new GLVertexArrayObject;
}

GRAPHICS_EXPORT VertexBufferObject* createVBO() {
	std::cout << "Creating the VBO\n";
	return new GLVertexBufferObject;
}

bool GraphicsWrapper::InitializeGraphics()
{
	//Initializing GL3W
	if (gl3wInit() != 0) {
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return true;
}

void GraphicsWrapper::DrawArrays(VertexArrayObject *vao, int start, unsigned int length)
{
	vao->Bind();
	glDrawArrays(GL_TRIANGLES, 0, 3);
	vao->Unbind();
}

void GraphicsWrapper::Clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
