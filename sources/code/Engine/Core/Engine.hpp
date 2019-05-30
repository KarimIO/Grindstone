#ifndef _ENGINE_H
#define _ENGINE_H

#include <vector>
#include <string>
#include <chrono>
#include "Utilities/SettingsFile.hpp"
#include "Systems/BaseSystem.hpp"
#include <VertexArrayObject.hpp>
#include <glm/glm.hpp>
#include <Texture.hpp>

class System;
class Scene;
class Settings;
class DLLGraphics;
class DLLAudio;
class GraphicsWrapper;
class AudioWrapper;

class AudioManager;
class ModelManager;
class MaterialManager;
class TextureManager;
class GraphicsPipelineManager;
class InputManager;

class UniformBufferBinding;
class UniformBuffer;

class GameObject;

typedef size_t SceneHandle;

class ImguiManager;

class Editor;

class Engine {
public:
	void initialize();
	void shutdown();

	void initializeUniformBuffer();
	void initializePlaneVertexBuffer();
	void deffUBO();
	void initializeTBL();

#ifdef INCLUDE_EDITOR
	void editorControl(double);
	void launchEditor();
	Editor *getEditor();
	void startSimulation();
	void stopSimulation();
	bool edit_mode_;
	bool edit_is_simulating_;
#endif

	TextureBindingLayout *gbuffer_tbl_;

	std::vector<TextureSubBinding> subbindings_;
	UniformBufferBinding *deff_ubb_;
	UniformBuffer *deff_ubo_handler_;

	struct DefferedUBO {
		glm::mat4 view;
		glm::mat4 invProj;
		glm::vec4 eyePos;
		glm::vec4 resolution;
		float time;
	};

	VertexArrayObject *getPlaneVAO();
	VertexBindingDescription getPlaneVBD();
	VertexAttributeDescription getPlaneVAD();

	VertexArrayObject *plane_vao_;
	VertexBuffer *plane_vbo_;
	VertexBindingDescription plane_vbd_;
	VertexAttributeDescription plane_vad_;

	UniformBuffer * getUniformBuffer();

	UniformBufferBinding *getUniformBufferBinding();

	static Engine &getInstance();

	Scene *addScene(std::string path);
	System *addSystem(System *system);
	System *getSystem(ComponentHandle type);
	std::vector<Scene *> &getScenes();
	Scene *getScene(SceneHandle scene);
	Scene *getScene(std::string scene);

	Settings *getSettings();

	GraphicsWrapper *getGraphicsWrapper();

	AudioManager *getAudioManager();
	MaterialManager *getMaterialManager();
	GraphicsPipelineManager *getGraphicsPipelineManager();
	TextureManager *getTextureManager();
	ModelManager *getModelManager();
	InputManager *getInputManager();
	ImguiManager *getImguiManager();

	void calculateTime();
	double getTimeCurrent();
	double getUpdateTimeDelta();

	void shutdownControl(double);

	void run();

	~Engine();
private:
	bool running_;

	System *systems_[NUM_COMPONENTS];
	std::vector<Scene *> simulate_scenes_;
	std::vector<Scene *> scenes_;

	Settings *settings_;

	GraphicsWrapper *graphics_wrapper_;
	AudioWrapper *audio_wrapper_;

	AudioManager *audio_manager_;
	MaterialManager *material_manager_;
	GraphicsPipelineManager *graphics_pipeline_manager_;
	TextureManager *texture_manager_;
	ModelManager *model_manager_;
	InputManager *input_manager_;
	ImguiManager *imgui_manager_;

#ifdef INCLUDE_EDITOR
	Editor *editor_;
#endif

	UniformBufferBinding *ubb_;
	UniformBuffer *ubo_;

	DLLGraphics *dll_graphics_;
	DLLAudio *dll_audio_;

	std::chrono::time_point<std::chrono::high_resolution_clock> current_time_, prev_time_, start_time_;
	std::chrono::nanoseconds delta_time_;

};

#define engine Engine::getInstance()

/*class Engine {
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
	void (*pfnDeleteGraphics)(GraphicsWrapper*);
	void LoadingScreenThread();

	bool InitializeGraphics();
	bool InitializeAudio();
	void InitializeSettings();

	void CheckModPaths();

	void RefreshContent(double);

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
	MainUBO ubo_buffer_;
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
	SAudio audio_system_;
	struct Settings {
		int resolutionX;
		int resolutionY;
		float fov;
		bool vsync;
		bool use_ssao;
		bool enableReflections;
		bool enableTesselation;
		bool enableShadows;
		bool showPipelineLoad;
		bool showMaterialLoad;
		bool showTextureLoad;
		float mouse_sensitivity;
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

	InputSystem &inputSystem;
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
	void playSound(double);
};*/

#endif