#include "pch.hpp"

#include <entt/entt.hpp>

#include "EngineCore.hpp"
#include "Logger.hpp"
#include "Profiling.hpp"
#include "ECS/SystemRegistrar.hpp"
#include "ECS/ComponentRegistrar.hpp"
#include "CoreComponents/setupCoreComponents.hpp"
#include "CoreSystems/setupCoreSystems.hpp"
#include "Scenes/Manager.hpp"
#include "PluginSystem/Manager.hpp"
#include "Events/InputManager.hpp"
#include "Common/Graphics/Core.hpp"
#include "Common/Display/DisplayManager.hpp"
#include "Common/Window/WindowManager.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include "Events/Dispatcher.hpp"

#include "EngineCore/Assets/Materials/MaterialManager.hpp"
#include "EngineCore/Assets/Textures/TextureManager.hpp"
#include "EngineCore/Assets/Shaders/ShaderManager.hpp"
#include "EngineCore/Assets/Mesh3d/Mesh3dManager.hpp"
#include "EngineCore/Assets/Mesh3d/Mesh3dRenderer.hpp"
#include "EngineCore/Assets/AssetRendererManager.hpp"

using namespace Grindstone;

bool EngineCore::Initialize(CreateInfo& createInfo) {
	Logger::init("../log/output.log");
	GRIND_PROFILE_BEGIN_SESSION("Loading", "../log/grind-profile-load.json");
	GRIND_LOG("Initializing {0}...", createInfo.applicationTitle);

	// Load core (Logging, ECS and Plugin Manager)
	pluginManager = new Plugins::Manager(this);
	pluginManager->load("PluginGraphicsOpenGL");

	eventDispatcher = new Events::Dispatcher();
	inputManager = new Input::Manager(eventDispatcher);

	Window::CreateInfo windowCreationInfo;
	windowCreationInfo.fullscreen = Window::FullscreenMode::Windowed;
	windowCreationInfo.title = "Sandbox";
	windowCreationInfo.width = 800;
	windowCreationInfo.height = 600;
	windowCreationInfo.engineCore = this;
	displayManager->GetMainDisplay();
	auto win = windowManager->Create(windowCreationInfo);

	GraphicsAPI::Core::CreateInfo graphicsCoreInfo{ win, true };
	graphicsCore->Initialize(graphicsCoreInfo);
	win->Show();

	materialManager = new MaterialManager();
	textureManager = new TextureManager();
	shaderManager = new ShaderManager();
	mesh3dManager = new Mesh3dManager();
	assetRendererManager = new AssetRendererManager();
	assetRendererManager->AddAssetRenderer(new Mesh3dRenderer());

	systemRegistrar = new ECS::SystemRegistrar();
	SetupCoreSystems(systemRegistrar);
	componentRegistrar = new ECS::ComponentRegistrar();
	SetupCoreComponents(componentRegistrar);
	sceneManager = new SceneManagement::SceneManager();

	sceneManager->LoadDefaultScene();

	GRIND_LOG("{0} Initialized.", createInfo.applicationTitle);
	GRIND_PROFILE_END_SESSION();

	return true;
}

EngineCore& EngineCore::GetInstance() {
	static EngineCore instance;
	return instance;
}

void EngineCore::Run() {
	while (!shouldClose) {
		RunLoopIteration();
		UpdateWindows();
	}
}

void EngineCore::RunLoopIteration() {
	sceneManager->Update();
}

void EngineCore::UpdateWindows() {
	windowManager->UpdateWindows();
	eventDispatcher->HandleEvents();
}

EngineCore::~EngineCore() {
	GRIND_LOG("Closing...");
	delete componentRegistrar;
	delete systemRegistrar;
	GRIND_LOG("Closed.");
}

void EngineCore::RegisterGraphicsCore(GraphicsAPI::Core* graphicsCore) {
	this->graphicsCore = graphicsCore;
}

void EngineCore::RegisterInputManager(Input::Interface* inputManager) {
	this->inputManager = inputManager;
}

Input::Interface* EngineCore::GetInputManager() {
	return inputManager;
}

SceneManagement::SceneManager* EngineCore::GetSceneManager() {
	return sceneManager;
}

ECS::ComponentRegistrar* EngineCore::GetComponentRegistrar() {
	return componentRegistrar;
}

GraphicsAPI::Core* EngineCore::GetGraphicsCore() {
	return graphicsCore;
}

ECS::SystemRegistrar* EngineCore::GetSystemRegistrar() {
	return systemRegistrar;
}

Events::Dispatcher* EngineCore::GetEventDispatcher() {
	return eventDispatcher;
}