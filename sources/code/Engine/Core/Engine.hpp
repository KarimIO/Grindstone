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

#include "../Core/Entity.hpp"

#include "../Systems/SPhysics.hpp"

#include "../Systems/STransform.hpp"
#include "../Systems/SCamera.hpp"
#include "../Systems/SController.hpp"
#include "../Systems/SMaterial.hpp"
#include "../Systems/SGameplay.hpp"
#include "../Systems/SUI.hpp"
#include "../Systems/Skybox.hpp"
#include "../Systems/SAudio.hpp"
#include "../Systems/Debug.hpp"
#include "exception.hpp"

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

class Entity;

class Engine {
public:
	TextureBindingLayout *tonemap_tbl_;
	std::string level_file_name_;
	std::string level_name_;
	Framebuffer * hdr_framebuffer_;
	RenderTarget *hdr_buffer_;
	RenderTargetContainer rt_gbuffer_;
	RenderTargetContainer rt_hdr_;
	RenderTargetContainer rt_ldr_;
	GraphicsPipeline *pipeline_bloom_;
	GraphicsPipeline *pipeline_ssr_;
	std::vector<TextureSubBinding> tbci_refl_;

	TextureBindingLayout *reflection_cubemap_layout_;

	DefferedUBO deffUBOBuffer;
	UniformBuffer *deffUBO;

	VertexArrayObject *planeVAO;
	VertexBuffer *planeVBO;
	VertexBindingDescription planeVBD;
	VertexAttributeDescription planeVAD;
	UniformBufferBinding *deffubb;
	UniformBufferBinding *lightubb;
	TextureBindingLayout *tbl;
	UniformBufferBinding *pointLightUBB;
	UniformBufferBinding *spotLightUBB;
	UniformBufferBinding *directionalLightUBB;
	std::vector<TextureSubBinding> bindings;
	Framebuffer *gbuffer_;
	Framebuffer *defaultFramebuffer;

	AudioWrapper *audio_wrapper_;
	SoundBuffer *sound_buffer_;
	SoundSource *sound_source_;
private:
	void (*pfnDeleteAudio)(AudioWrapper*);
	void LoadingScreenThread();

	bool InitializeGraphics();
	bool InitializeAudio();
	void InitializeSettings();

	void CheckModPaths();

	RenderPathType renderPathType;
	RenderPath *renderPath;
	PostPipeline postPipeline;

	//SUI sUi;

	// Time Data
	std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, prevTime, startTime;
	double accumulator = 0.0;

	std::vector<std::string> modPaths;

	std::chrono::nanoseconds deltaTime;
public:
	UniformBuffer * ubo;
	UniformBuffer *ubo2;

	GraphicsPipeline *pipeline;
	Debug debug_wrapper_;
	Skybox skybox_;
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
		bool use_ssao;
		bool enableReflections;
		bool enableShadows;
		bool showPipelineLoad;
		bool showMaterialLoad;
		bool showTextureLoad;
		GraphicsLanguage graphicsLanguage;
	} settings;

	std::string defaultMap;

	std::vector<Entity> entities;

	CubemapSystem cubemapSystem;
	InputComponent input;

	SGeometry geometry_system;
	SLight lightSystem;
	DepthTarget *depth_image_;
	RenderTarget *gbuffer_images_;

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
	void PlaySound(double);
};

#define engine Engine::GetInstance()

#endif