#ifndef _ENGINE_H
#define _ENGINE_H

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <OGLGraphicsWrapper.h>
#include "BaseClass.h"
#include "InputInterface.h"
#include <Window.h>
#include <GLShader.h>
#include "SGeometry.h"
#include "Input.h"

#include <chrono>
#include <string>

class Engine : public BaseClass {
private:
	bool InitializeWindow();
	bool InitializeGraphics();

	// Remove this ASAP
	ShaderProgram *shader;
	VertexArrayObject *vao;
	VertexBufferObject *vbo;

	// Time Data
	std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, prevTime;

	std::chrono::microseconds deltaTime;
public:
	GameWindow *window;
	SModel geometryCache;
	//InputSystem inputSystem;

	// Input
	glm::vec3 position;
	glm::vec3 angles;
	glm::vec3 getForward();
	glm::vec3 getRight();
	glm::vec3 getUp();

	InputInterface inputInterface;
	GraphicsWrapper *graphicsWrapper;
	//InputSystem *inputSystem;
#ifdef UseClassInstance
	static Engine *GetInstance();
#else
	static Engine &GetInstance();
#endif
	bool Initialize();
	void Run();

	std::string GetAvailablePath(std::string);

	bool InitializeScene(std::string);
	void CalculateTime();
	double GetTimeCurrent();
	double GetTimeDelta();

	void Shutdown();

	bool isRunning;

	~Engine();
};

#ifdef UseClassInstance
	#define engine (*Engine::GetInstance())	// Needs work
#else
	#define engine Engine::GetInstance()	// Only works with C++11+
#endif

#endif