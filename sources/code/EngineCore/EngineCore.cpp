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
#include "Events/Dispatcher.hpp"
#include "Common/Event/WindowEvent.hpp"

#include "Rendering/DeferredRenderer.hpp"

#include "EngineCore/Assets/Materials/MaterialManager.hpp"
#include "EngineCore/Assets/Textures/TextureManager.hpp"
#include "EngineCore/Assets/Shaders/ShaderManager.hpp"
#include "EngineCore/Assets/Mesh3d/Mesh3dManager.hpp"
#include "EngineCore/Assets/Mesh3d/Mesh3dRenderer.hpp"
#include "EngineCore/Assets/AssetRendererManager.hpp"
#include "EngineCore/Audio/AudioCore.hpp"

using namespace Grindstone;

bool EngineCore::Initialize(CreateInfo& createInfo) {
	eventDispatcher = new Events::Dispatcher();

	Logger::Initialize("../log/output.log");
	GRIND_PROFILE_BEGIN_SESSION("Loading", "../log/grind-profile-load.json");
	Logger::Print("Initializing {0}...", createInfo.applicationTitle);

	// Load core (Logging, ECS and Plugin Manager)
	pluginManager = new Plugins::Manager(this);
	pluginManager->Load("PluginGraphicsOpenGL");

	inputManager = new Input::Manager(eventDispatcher);

	Window::CreateInfo windowCreationInfo;
	windowCreationInfo.fullscreen = Window::FullscreenMode::Windowed;
	windowCreationInfo.title = "Sandbox";
	windowCreationInfo.width = 800;
	windowCreationInfo.height = 600;
	windowCreationInfo.engineCore = this;
	auto win = windowManager->Create(windowCreationInfo);
	eventDispatcher->AddEventListener(Grindstone::Events::EventType::WindowTryQuit, std::bind(&EngineCore::OnTryQuit, this, std::placeholders::_1));
	eventDispatcher->AddEventListener(Grindstone::Events::EventType::WindowTryQuit, std::bind(&EngineCore::OnForceQuit, this, std::placeholders::_1));

	GraphicsAPI::Core::CreateInfo graphicsCoreInfo{ win, true };
	graphicsCore->Initialize(graphicsCoreInfo);
	win->Show();

	audioCore = new Audio::Core();
	materialManager = new MaterialManager();
	textureManager = new TextureManager();
	shaderManager = new ShaderManager();
	mesh3dManager = new Mesh3dManager();
	mesh3dRenderer = new Mesh3dRenderer();
	assetRendererManager = new AssetRendererManager();
	assetRendererManager->AddAssetRenderer(mesh3dRenderer);
	assetRendererManager->AddQueue("Opaque");
	assetRendererManager->AddQueue("Transparent");
	assetRendererManager->AddQueue("Unlit");
	mesh3dRenderer->AddErrorMaterial();

	systemRegistrar = new ECS::SystemRegistrar();
	SetupCoreSystems(systemRegistrar);
	componentRegistrar = new ECS::ComponentRegistrar();
	SetupCoreComponents(componentRegistrar);
	sceneManager = new SceneManagement::SceneManager();
	pluginManager->SetupManagers();

	pluginManager->Load("PluginBulletPhysics");

	sceneManager->LoadDefaultScene();

	Logger::Print("{0} Initialized.", createInfo.applicationTitle);
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

void EngineCore::RunEditorLoopIteration() {
	CalculateDeltaTime();
	sceneManager->EditorUpdate();
}

void EngineCore::RunLoopIteration() {
	CalculateDeltaTime();
	sceneManager->Update();
}

void EngineCore::UpdateWindows() {
	windowManager->UpdateWindows();
	eventDispatcher->HandleEvents();
}

EngineCore::~EngineCore() {
	Logger::Print("Closing...");
	delete componentRegistrar;
	delete systemRegistrar;
	Logger::Print("Closed.");
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

BaseRenderer* EngineCore::CreateRenderer() {
	return new DeferredRenderer();
}

bool EngineCore::OnTryQuit(Grindstone::Events::BaseEvent* ev) {
	auto castedEv = (Grindstone::Events::WindowTryQuitEvent*)ev;
	shouldClose = true;
	windowManager->CloseWindow(castedEv->window);

	return false;
}

bool EngineCore::OnForceQuit(Grindstone::Events::BaseEvent* ev) {
	auto castedEv = (Grindstone::Events::WindowTryQuitEvent*)ev;
	shouldClose = true;
	windowManager->CloseWindow(castedEv->window);

	return false;
}

void EngineCore::CalculateDeltaTime() {
	auto now = std::chrono::steady_clock::now();
	auto elapsedTime = now - lastFrameTime;
	auto elapsedNs = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(elapsedTime).count();
	deltaTime = elapsedNs * 0.000000001;

	lastFrameTime = now;
}

double EngineCore::GetDeltaTime() {
	return deltaTime;
}

void EngineCore::Print(LogSeverity logSeverity, const char* textFormat, ...) {
	va_list args;
	va_start(args, textFormat);
	Logger::Print(logSeverity, textFormat, args);
	va_end(args);
}
