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

#include "SCubemap.h"

#include <chrono>
#include <string>

enum RenderPathType {
	RENDERPATH_FORWARD = 0,
	RENDERPATH_DEFERRED,
}; // Also, F+, TileBased, etc

enum GraphicsLanguage {
	GRAPHICS_OPENGL = 0,
	GRAPHICS_VULKAN,
	GRAPHICS_DIRECTX,
	GRAPHICS_METAL
};

class Engine : public BaseClass {
private:
	bool InitializeWindow();
	bool InitializeGraphics(GraphicsLanguage);
	void InitializeSettings();

	RenderPathType renderPathType;
	RenderPath *renderPath;

	EBasePlayer *player;

	// Time Data
	std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, prevTime, startTime;
	double accumulator = 0.0;

	std::chrono::nanoseconds deltaTime;
public:
	// Remove this ASAP
	ShaderProgram *shader;

	struct Settings {
		int resolutionX;
		int resolutionY;
		float fov;
		GraphicsLanguage graphicsLanguage;
	} settings;

	std::vector<EBase> entities;

	CubemapSystem cubemapSystem;
	InputComponent input;

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
	double GetUpdateTimeDelta();
	double GetRenderTimeDelta();
	void Render(glm::mat4 projection, glm::mat4 views, glm::vec2 res);

	void Shutdown();

	bool isRunning;

	~Engine();
	void ShutdownControl(double);
};

#ifdef UseClassInstance
	#define engine (*Engine::GetInstance())	// Needs work
#else
	#define engine Engine::GetInstance()	// Only works with C++11+
#endif

#endif