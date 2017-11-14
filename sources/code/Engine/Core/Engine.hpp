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

#include "../Systems/SGeometry.h"
#include "Input.h"

#include "../Systems/SCubemap.h"
#include "../Systems/SLight.h"

#include <chrono>
#include <string>

#include <SoundFile.h>

#include "../Core/Entity.h"

#include "../Systems/SPhysics.h"

#include "../Systems/STransform.h"
#include "../Systems/SCamera.h"
#include "../Systems/SController.h"
#include "Systems/SMaterial.h"
#include "Systems/SGameplay.h"
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
class Entity;

struct MatUniformBufferObject {
	glm::mat4 view;
	glm::mat4 proj;
};

struct ModelMatrixUBO {
	glm::mat4 model;
};

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
	MatUniformBufferObject myUBO;

	// Time Data
	std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, prevTime, startTime;
	double accumulator = 0.0;

	std::vector<std::string> modPaths;

	std::chrono::nanoseconds deltaTime;
public:
	UniformBuffer *ubo2;
	ModelMatrixUBO modelUBO;

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
	void Render(glm::mat4, glm::mat4);

	void Shutdown();

	bool isRunning;

	~Engine();
	void ShutdownControl(double);
};

#define engine Engine::GetInstance()

#endif