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
#include "../Entities/EBasePlayer.h"

#include "../Systems/SCubemap.h"
#include "../Systems/SLight.h"

#include <chrono>
#include <string>

#include <SoundFile.h>

#include <Shader.h>

#include "../Entities/EBase.h"

#include "../Systems/Physics.h"

struct classRegister {
	std::string entityName;
	std::function<void *()> function;
	classRegister(std::string str, std::function<void *()> fn) : entityName(str), function(fn) {}
};


#define LINK_ENTITY_TO_CLASS(NAME, CLASS) \
class __Register_##CLASS { \
public: \
	static void *CreateNew() { \
		return new CLASS(); \
	} \
	__Register_##CLASS() { \
		engine.classRegistry.push_back(new classRegister(NAME, &__Register_##CLASS::CreateNew)); \
	} \
}; \
__Register_##CLASS __register_##CLASS;

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

class EBase;

class Engine {
private:
	bool InitializeWindow();
	bool InitializeAudio();
	bool InitializeGraphics(GraphicsLanguage);
	void InitializeSettings();

	RenderPathType renderPathType;
	RenderPath *renderPath;

	PhysicsSystem physics;
	std::vector<System *> systems;

	AudioSystem *audioSystem;
	std::vector<SoundFile *> sounds;

	// Time Data
	std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, prevTime, startTime;
	double accumulator = 0.0;

	std::chrono::nanoseconds deltaTime;
public:
	// Class Registry
	std::vector<classRegister *> classRegistry;
	EBase *createEntity(const char *szEntityName);
	void registerClass(const char *szEntityName, std::function<void *()> fn);
	EBasePlayer *player;
	struct Settings {
		int resolutionX;
		int resolutionY;
		float fov;
		bool enableReflections;
		bool enableShadows;
		GraphicsLanguage graphicsLanguage;
	} settings;

	std::vector<EBase> entities;

	CubemapSystem cubemapSystem;
	InputComponent input;

	GameWindow *window;
	SModel geometryCache;
	SLight lightSystem;

	ShaderProgram *shader;

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

	bool InitializeScene(std::string);
	void CalculateTime();
	double GetTimeCurrent();
	double GetUpdateTimeDelta();
	double GetRenderTimeDelta();
	void Render(glm::mat4 projection, glm::mat4 views, bool usePost);
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