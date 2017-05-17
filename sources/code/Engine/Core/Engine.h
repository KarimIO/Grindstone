#ifndef _ENGINE_H
#define _ENGINE_H

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <GraphicsWrapper.h>

#include <AudioCommon.h>

#include "../Renderpaths/RenderPath.h"
#include "../Renderpaths/RenderPathDeferred.h"
#include "../Renderpaths/RenderPathForward.h"

#include <Window.h>
#include "Shader.h"
#include "../Systems/SGeometry.h"
#include "Input.h"

#include "../Systems/SCubemap.h"
#include "../Systems/SLight.h"

#include <chrono>
#include <string>

#include <SoundFile.h>

#include "TextureManager.h"

#include <Shader.h>

#include "../Core/EBase.h"

#include "../Systems/Physics.h"

#include "../Systems/CTransform.h"
#include "../Systems/CCamera.h"
#include "../Systems/CController.h"

#include "Systems/SUI.h"

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

enum {
	DEBUG_NONE = 0,
	DEBUG_FOURDISPLAY,
	DEBUG_DEPTH,
	DEBUG_POSITION,
	DEBUG_2DNORMAL,
	DEBUG_NORMAL,
	DEBUG_ALBEDO,
	DEBUG_SPECULAR,
	DEBUG_ROUGHNESS,
	DEBUG_SHADOW,
	NUM_DEBUG
};
#include "PostProcess/PostPipeline.h"
class EBase;

class Engine {
private:
	bool InitializeWindow();
	bool InitializeAudio();
	bool InitializeGraphics(GraphicsLanguage);
	void InitializeSettings();

	void LoadShadowShader();
	void LoadMainShader();

	void CheckModPaths();

	RenderPathType renderPathType;
	RenderPath *renderPath;
	PostPipeline postPipeline;

	//SUI sUi;

	std::vector<System *> systems;

	AudioSystem *audioSystem;
	std::vector<SoundFile *> sounds;

	// Time Data
	std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, prevTime, startTime;
	double accumulator = 0.0;

	std::vector<std::string> modPaths;

	std::chrono::nanoseconds deltaTime;
public:
	VertexArrayObject	*vaoQuad;
	VertexBufferObject	*vboQuad;

	int debugMode;
	TextureManager textureManager;
	STerrain terrainSystem;
	STransform transformSystem;
	SCamera cameraSystem;
	SController controllerSystem;
	SPhysics physicsSystem;
	struct Settings {
		int resolutionX;
		int resolutionY;
		float fov;
		bool enableReflections;
		bool enableShadows;
		GraphicsLanguage graphicsLanguage;
	} settings;

	std::string defaultMap;

	std::vector<EBase> entities;

	CubemapSystem cubemapSystem;
	InputComponent input;

	GameWindow *window;
	SModel geometryCache;
	SLight lightSystem;

	ShaderProgram *shader;
	ShaderProgram *shadowShader;

	InputSystem inputSystem;
	GraphicsWrapper *graphicsWrapper;
#ifdef UseClassInstance
	static Engine *GetInstance();
#else
	static Engine &GetInstance();
#endif
	bool Initialize();
	void Run();

	//void AddSystem(System *newSystem);
	//void AddSpace(Space *newSpace);

	std::string GetAvailablePath(std::string);

	void SwitchDebug(double);

	bool InitializeScene(std::string);
	void CalculateTime();
	double GetTimeCurrent();
	double GetUpdateTimeDelta();
	double GetRenderTimeDelta();
	void Render(glm::mat4, glm::mat4, glm::vec3, bool);
	void PlayEngineSound(double sound);
	void PlayEngineSound2(double sound);

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