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

#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "EngineCore/Assets/Textures/TextureImporter.hpp"
#include "EngineCore/Assets/Shaders/ShaderImporter.hpp"
#include "EngineCore/Assets/Mesh3d/Mesh3dImporter.hpp"
#include "EngineCore/Assets/Mesh3d/Mesh3dRenderer.hpp"
#include "EngineCore/Assets/AssetRendererManager.hpp"

using namespace Grindstone;

bool EngineCore::Initialize(CreateInfo& createInfo) {
	projectPath = createInfo.projectPath;
	binaryPath = std::string(createInfo.projectPath) + "/bin/";
	assetsPath = std::string(createInfo.projectPath) + "/compiledAssets/";
	eventDispatcher = new Events::Dispatcher();

	firstFrameTime = std::chrono::steady_clock::now();

	Logger::Initialize("../log/output.log");
	GRIND_PROFILE_BEGIN_SESSION("Loading", "../log/grind-profile-load.json");
	Logger::Print("Initializing {0}...", createInfo.applicationTitle);

	systemRegistrar = new ECS::SystemRegistrar();
	SetupCoreSystems(systemRegistrar);
	componentRegistrar = new ECS::ComponentRegistrar();
	SetupCoreComponents(componentRegistrar);
	sceneManager = new SceneManagement::SceneManager();

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

	materialImporter = new MaterialImporter();
	textureImporter = new TextureImporter();
	shaderImporter = new ShaderImporter();
	mesh3dImporter = new Mesh3dImporter();
	mesh3dRenderer = new Mesh3dRenderer();
	assetRendererManager = new AssetRendererManager();
	assetRendererManager->AddAssetRenderer(mesh3dRenderer);
	assetRendererManager->AddQueue("Opaque");
	assetRendererManager->AddQueue("Transparent");
	assetRendererManager->AddQueue("Unlit");

	pluginManager->LoadPluginList();

	if (createInfo.shouldLoadSceneFromDefaults) {
		sceneManager->LoadDefaultScene();
	}
	else if (strcmp(createInfo.scenePath, "") == 0) {
		sceneManager->AddEmptyScene("Untitled");
	}
	else {
		sceneManager->LoadScene(createInfo.scenePath);
	}

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

Plugins::Manager* EngineCore::GetPluginManager() {
	return pluginManager;
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

std::filesystem::path EngineCore::GetProjectPath() {
	return projectPath;
}

std::filesystem::path EngineCore::GetBinaryPath() {
	return binaryPath;
}

std::filesystem::path EngineCore::GetAssetsPath() {
	return assetsPath;
}

std::filesystem::path EngineCore::GetAssetPath(std::string subPath) {
	return assetsPath / subPath;
}

void EngineCore::Print(LogSeverity logSeverity, const char* str) {
	Logger::Print(logSeverity, str);
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

	auto elapsedTimeSinceLastFrame = now - lastFrameTime;
	auto elapsedNsSinceLastFrame = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(elapsedTimeSinceLastFrame).count();
	deltaTime = elapsedNsSinceLastFrame * 0.000000001;

	auto elapsedTimeSinceFirstTime = now - lastFrameTime;
	auto elapsedNsSinceFirstTime = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(now - firstFrameTime).count();
	currentTime = elapsedNsSinceFirstTime * 0.000000001;

	lastFrameTime = now;
}

double EngineCore::GetTimeSinceLaunch() {
	return currentTime;
}

double EngineCore::GetDeltaTime() {
	return deltaTime;
}

extern "C" {
	ENGINE_CORE_API double TimeGetTimeSinceLaunch() {
		return EngineCore::GetInstance().GetTimeSinceLaunch();
	}

	ENGINE_CORE_API double TimeGetDeltaTime() {
		return EngineCore::GetInstance().GetDeltaTime();
	}
}
