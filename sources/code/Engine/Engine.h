#ifndef _ENGINE_H
#define _ENGINE_H

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <GraphicsWrapper.h>

#include "RenderPath.h"
#include "RenderPathDeferred.h"
#include "RenderPathForward.h"

#include "BaseClass.h"
#include <Window.h>
#include <GLShader.h>
#include "SGeometry.h"
#include "Input.h"
#include "EBasePlayer.h"

#include <chrono>
#include <string>

enum RenderPathType {
	RENDERPATH_FORWARD = 0,
	RENDERPATH_DEFERRED,
}; // Also, F+, TileBased, etc

class Engine : public BaseClass {
private:
	bool InitializeWindow();
	bool InitializeGraphics();

	// Remove this ASAP
	ShaderProgram *shader;
	VertexArrayObject *vao;
	VertexBufferObject *vbo;

	RenderPathType renderPathType;
	RenderPath *renderPath;

	EBasePlayer *player;

	// Time Data
	std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, prevTime, startTime;

	std::chrono::milliseconds deltaTime;
public:
	struct Settings {
		int resolutionX;
		int resolutionY;
	} settings;

	GameWindow *window;
	SModel geometryCache;

	InputSystem inputSystem;
	GraphicsWrapper *graphicsWrapper;
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