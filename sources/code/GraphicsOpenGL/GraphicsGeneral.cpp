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

	// An array of 3 vectors which represents 3 vertices
	GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
	};

	/*GLVertexArrayObject vao;
	vao.Initialize();

	GLVertexBufferObject vbo;
	vbo.Initialize(1);
	vbo.AddVBO(g_vertex_buffer_data, (uint64_t)sizeof(g_vertex_buffer_data), (uint8_t)0, (uint8_t)0);

	vao.BindVBO(&vbo);*/

	GLVertexArrayObject vao;
	vao.Initialize();
	vao.Bind();

	GLVertexBufferObject vbo;
	vbo.Initialize(1);
	vbo.AddVBO(g_vertex_buffer_data, (uint64_t)sizeof(g_vertex_buffer_data), (uint8_t)0, (uint8_t)0);
	vbo.Bind(0);
	//vao.BindVBO(&vbo);

	/*glBindVertexArray(VertexArrayID);
	glEnableVertexAttribArray(0);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
		);
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);*/
	vao.Unbind();

	vao.Bind();
	//glBindVertexArray(VertexArrayID);
	glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
	//glBindVertexArray(0);
	vao.Unbind();
	//vao.Unbind();

	return true;
}