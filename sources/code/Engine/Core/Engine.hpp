#ifndef _ENGINE_H
#define _ENGINE_H

#include <VertexBuffer.hpp>
#include <Texture.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GraphicsWrapper;
		class GraphicsPipeline;
		class CommandBuffer;
		class Texture;
		class TextureBindingLayout;
		class RenderPass;
		class Framebuffer;
		class UniformBufferBinding;
		class VertexArrayObject;
	}
}

class System;
class Scene;
class Settings;
class DLLGraphics;
class DLLAudio;
class AudioWrapper;

class AudioManager;
class ModelManager;
class MaterialManager;
class TextureManager;
class GraphicsPipelineManager;
class InputManager;

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

	void consoleCommand(std::string command);

#ifdef INCLUDE_EDITOR
	void editorControl(double);
	void launchEditor();
	Editor *getEditor();
	void startSimulation();
	void stopSimulation();
	bool edit_mode_;
	bool edit_is_simulating_;
#endif

	Grindstone::GraphicsAPI::TextureBindingLayout *gbuffer_tbl_;

	std::vector<Grindstone::GraphicsAPI::TextureSubBinding> subbindings_;
	Grindstone::GraphicsAPI::UniformBufferBinding *deff_ubb_;
	Grindstone::GraphicsAPI::UniformBuffer *deff_ubo_handler_;

	struct DefferedUBO {
		glm::mat4 view;
		glm::mat4 invProj;
		glm::vec4 eyePos;
		glm::vec4 resolution;
		float time;
	};

	Grindstone::GraphicsAPI::VertexArrayObject *getPlaneVAO();
	Grindstone::GraphicsAPI::VertexBindingDescription getPlaneVBD();
	Grindstone::GraphicsAPI::VertexAttributeDescription getPlaneVAD();

	Grindstone::GraphicsAPI::VertexArrayObject *plane_vao_;
	Grindstone::GraphicsAPI::VertexBuffer *plane_vbo_;
	Grindstone::GraphicsAPI::VertexBindingDescription plane_vbd_;
	Grindstone::GraphicsAPI::VertexAttributeDescription plane_vad_;

	Grindstone::GraphicsAPI::UniformBuffer * getUniformBuffer();

	Grindstone::GraphicsAPI::UniformBufferBinding *getUniformBufferBinding();

	static Engine &getInstance();

	Scene *addScene(std::string path);

	template<class T>
	T *getSystem() {
		return (T *)(systems_[T::static_system_type_]);
	}

	System *addSystem(System *system);
	System *getSystem(ComponentHandle type);
	std::vector<Scene *> &getScenes();
	Scene *getScene(SceneHandle scene);
	Scene *getScene(std::string scene);

	Settings *getSettings();

	Grindstone::GraphicsAPI::GraphicsWrapper *getGraphicsWrapper();

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

	void reloadAudioDLL();

	void reloadGraphicsDLL();

	void refreshAll(double);
	void profileFrame(double);

	void run();

	~Engine();
private:
	bool running_;
	bool profile_frame_;

	System *systems_[NUM_COMPONENTS];
	std::vector<Scene *> simulate_scenes_;
	std::vector<Scene *> scenes_;

	Settings *settings_;

	Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper_;
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

	Grindstone::GraphicsAPI::UniformBufferBinding *ubb_;
	Grindstone::GraphicsAPI::UniformBuffer *ubo_;

	DLLGraphics *dll_graphics_;
	DLLAudio *dll_audio_;

	std::chrono::time_point<std::chrono::high_resolution_clock> current_time_, prev_time_, start_time_;
	std::chrono::nanoseconds delta_time_;

};

#define engine Engine::getInstance()

#endif