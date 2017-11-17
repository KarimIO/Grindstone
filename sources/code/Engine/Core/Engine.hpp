#ifndef _ENGINE_H
#define _ENGINE_H

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <GraphicsWrapper.hpp>

#include <AudioCommon.hpp>

#include "../Renderpaths/RenderPath.hpp"
#include "../Renderpaths/RenderPathDeferred.hpp"
#include "../Renderpaths/RenderPathForward.hpp"

#include "../Systems/SGeometry.hpp"
#include "Input.hpp"

#include "../Systems/SCubemap.hpp"
#include "../Systems/SLight.hpp"

#include <chrono>
#include <string>

#include <SoundFile.hpp>

#include "../Core/Entity.hpp"

#include "../Systems/SPhysics.hpp"

#include "../Systems/STransform.hpp"
#include "../Systems/SCamera.hpp"
#include "../Systems/SController.hpp"
#include "Systems/SMaterial.hpp"
#include "Systems/SGameplay.hpp"
#include "Systems/SUI.hpp"

#include "PostProcess/PostPipeline.hpp"
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
class Entity;

class Engine {
public:
	VertexBindingDescription planeVBD;
	VertexAttributeDescription planeVAD;
	UniformBufferBinding *deffubb;
	UniformBufferBinding *lightubb;
	TextureBindingLayout *tbl;
	UniformBufferBinding *pointLightUBB;
	UniformBufferBinding *spotLightUBB;
	std::vector<TextureSubBinding> bindings;
private:
	void LoadingScreenThread();
	Framebuffer *gbuffer;
	Framebuffer *defaultFramebuffer;

	bool InitializeGraphics(GraphicsLanguage);
	void InitializeSettings();

	void CheckModPaths();

	RenderPathType renderPathType;
	RenderPath *renderPath;
	PostPipeline postPipeline;

	//SUI sUi;
	UniformBuffer *ubo;

	// Time Data
	std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, prevTime, startTime;
	double accumulator = 0.0;

	std::vector<std::string> modPaths;

	std::chrono::nanoseconds deltaTime;
public:
	UniformBuffer *ubo2;

	GraphicsPipeline *pipeline;
	int debugMode;
	MaterialManager materialManager;
	STransform transformSystem;
	SCamera cameraSystem;
	SController controllerSystem;
	SPhysics physicsSystem;
	SGameplay gameplay_system;
	struct Settings {
		int resolutionX;
		int resolutionY;
		float fov;
		bool vsync;
		bool enableReflections;
		bool enableShadows;
		bool debugNoLighting;
		GraphicsLanguage graphicsLanguage;
	} settings;

	std::string defaultMap;

	std::vector<Entity> entities;

	CubemapSystem cubemapSystem;
	InputComponent input;

	SGeometry geometry_system;
	SLight lightSystem;

	RenderPass *renderPass;
	std::vector<Framebuffer *> fbos;

	InputSystem inputSystem;
	GraphicsWrapper *graphics_wrapper_;

	static Engine &GetInstance();

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
	void Render();

	void Shutdown();

	bool isRunning;

	~Engine();
	void ShutdownControl(double);
};

#define engine Engine::GetInstance()

#endif