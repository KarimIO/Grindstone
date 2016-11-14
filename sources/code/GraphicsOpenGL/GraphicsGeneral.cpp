#include "OpenGLGraphics.h"
#include "gl3w.h"

GRAPHICS_EXPORT GraphicsWrapper* createGraphics() {
	return new GraphicsWrapper;
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
}