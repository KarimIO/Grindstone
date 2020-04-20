#ifndef _ENGINE_H
#define _ENGINE_H

#include "../pch.hpp"

#include <GraphicsCommon/GraphicsWrapper.hpp>
#include <GraphicsCommon/UniformBuffer.hpp>
#include <GraphicsCommon/VertexBuffer.hpp>
#include <GraphicsCommon/Texture.hpp>
#include <Engine/Systems/BaseSystem.hpp>
#include <WindowModule/BaseWindow.hpp>

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

class Space;
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

class BaseWindow;

typedef size_t SceneHandle;

#ifdef _MSC_VER
#ifdef GRIND_ENGINE_DLL
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif
#else
#define ENGINE_API
#endif

class Engine {
public:
	ENGINE_API virtual void initialize(BaseWindow *window);
	ENGINE_API virtual void shutdown();
	ENGINE_API virtual void run();
	ENGINE_API virtual bool shouldQuit();

	void consoleCommand(std::string command);

public:
	Grindstone::GraphicsAPI::VertexArrayObject *getPlaneVAO();
	Grindstone::GraphicsAPI::VertexBufferLayout getPlaneVertexLayout();
	Grindstone::GraphicsAPI::UniformBuffer * getUniformBuffer();
	Grindstone::GraphicsAPI::UniformBufferBinding *getUniformBufferBinding();
	Grindstone::GraphicsAPI::TextureBindingLayout* getGbufferTBL();

	static Engine &getInstance();

	ENGINE_API virtual Space *addSpace(const char *path);

	template<class T>
	T *getSystem() {
		return (T *)(systems_[T::static_system_type_]);
	}

	System *addSystem(System *system);
	System *getSystem(ComponentHandle type);
	std::vector<Space*> &getSpaces();
	Space *getSpace(size_t scene);
	Space *getSpace(std::string scene);

	Settings *getSettings();

	BaseWindow* getWindow();
	Grindstone::GraphicsAPI::GraphicsWrapper* getGraphicsWrapper();

	AudioManager *getAudioManager();
	MaterialManager *getMaterialManager();
	GraphicsPipelineManager *getGraphicsPipelineManager();
	TextureManager *getTextureManager();
	ModelManager *getModelManager();
	InputManager *getInputManager();

	void calculateTime();
	double getTimeCurrent();
	double getUpdateTimeDelta();

	void shutdownControl(double);

	void reloadAudioDLL();

	void reloadGraphicsDLL();

	void refreshAll(double);
	void profileFrame(double);

	~Engine();

	struct DefferedUBO {
		glm::mat4 view;
		glm::mat4 invProj;
		glm::vec4 eyePos;
		glm::vec4 resolution;
		float time;
	};

	Grindstone::GraphicsAPI::UniformBufferBinding* deff_ubb_;
	Grindstone::GraphicsAPI::UniformBuffer* deff_ubo_handler_;

private:
	void initializeUniformBuffer();
	void initializePlaneVertexBuffer();
	void deffUBO();
	void initializeTBL();

	Grindstone::GraphicsAPI::TextureBindingLayout* gbuffer_tbl_;

	std::vector<Grindstone::GraphicsAPI::TextureSubBinding> subbindings_;

	Grindstone::GraphicsAPI::VertexArrayObject* plane_vao_;
	Grindstone::GraphicsAPI::VertexBuffer* plane_vbo_;
	Grindstone::GraphicsAPI::VertexBufferLayout plane_vertex_layout_;

private:
	bool running_;
	bool profile_frame_;

	System *systems_[NUM_COMPONENTS];
	std::vector<Space *> spaces_;

	Settings *settings_;

	BaseWindow *window_;
	Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper_;
	AudioWrapper *audio_wrapper_;

	AudioManager *audio_manager_;
	MaterialManager *material_manager_;
	GraphicsPipelineManager *graphics_pipeline_manager_;
	TextureManager *texture_manager_;
	ModelManager *model_manager_;
	InputManager *input_manager_;

	Grindstone::GraphicsAPI::UniformBufferBinding *ubb_;
	Grindstone::GraphicsAPI::UniformBuffer *ubo_;

	DLLGraphics *dll_graphics_;
	DLLAudio *dll_audio_;

	std::chrono::time_point<std::chrono::high_resolution_clock> current_time_, prev_time_, start_time_;
	std::chrono::nanoseconds delta_time_;

};

inline Settings* Engine::getSettings() {
	return settings_;
}

inline BaseWindow* Engine::getWindow() {
	return window_;
}

inline Grindstone::GraphicsAPI::GraphicsWrapper* Engine::getGraphicsWrapper() {
	return graphics_wrapper_;
}

inline AudioManager* Engine::getAudioManager() {
	return audio_manager_;
}

inline MaterialManager* Engine::getMaterialManager() {
	return material_manager_;
}

inline GraphicsPipelineManager* Engine::getGraphicsPipelineManager() {
	return graphics_pipeline_manager_;
}

inline TextureManager* Engine::getTextureManager() {
	return texture_manager_;
}

inline ModelManager* Engine::getModelManager() {
	return model_manager_;
}

inline InputManager* Engine::getInputManager() {
	return input_manager_;
}

#define engine Engine::getInstance()

extern "C" {
	ENGINE_API void* launchEngine();
	ENGINE_API void deleteEngine(void* ptr);
}

#endif