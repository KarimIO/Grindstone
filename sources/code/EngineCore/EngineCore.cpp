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

#include "EngineCore/AssetRenderer/AssetRendererManager.hpp"
#include "Assets/AssetManager.hpp"

using namespace Grindstone;

bool EngineCore::Initialize(CreateInfo& createInfo) {
	projectPath = createInfo.projectPath;
	binaryPath = projectPath / "bin/";
	assetsPath = projectPath / "compiledAssets/";
	eventDispatcher = new Events::Dispatcher();

	firstFrameTime = std::chrono::steady_clock::now();

	Logger::Initialize(projectPath / "log/output.log");
	GRIND_PROFILE_BEGIN_SESSION("Grindstone Loading", projectPath / "log/grind-profile-load.json");
	Logger::Print("Initializing {0}...", createInfo.applicationTitle);

	{
		GRIND_PROFILE_SCOPE("Setup Core Systems");
		systemRegistrar = new ECS::SystemRegistrar();
		SetupCoreSystems(systemRegistrar);
	}

	{
		GRIND_PROFILE_SCOPE("Setup Core Components");
		componentRegistrar = new ECS::ComponentRegistrar();
		SetupCoreComponents(componentRegistrar);
	}

	// Load core (Logging, ECS and Plugin Manager)
	pluginManager = new Plugins::Manager(this);
	pluginManager->Load("PluginGraphicsOpenGL");

	Grindstone::Window* win = nullptr;
	{
		GRIND_PROFILE_SCOPE("Set up InputManager and Window");
		inputManager = new Input::Manager(eventDispatcher);

		Window::CreateInfo windowCreationInfo;
		windowCreationInfo.fullscreen = Window::FullscreenMode::Windowed;
		windowCreationInfo.title = "Sandbox";
		windowCreationInfo.width = 800;
		windowCreationInfo.height = 600;
		windowCreationInfo.engineCore = this;
		win = windowManager->Create(windowCreationInfo);
		eventDispatcher->AddEventListener(Grindstone::Events::EventType::WindowTryQuit, std::bind(&EngineCore::OnTryQuit, this, std::placeholders::_1));
		eventDispatcher->AddEventListener(Grindstone::Events::EventType::WindowForceQuit, std::bind(&EngineCore::OnForceQuit, this, std::placeholders::_1));
	}

	{
		GRIND_PROFILE_SCOPE("Initialize Graphics Core");
		GraphicsAPI::Core::CreateInfo graphicsCoreInfo{ win, true };
		graphicsCore->Initialize(graphicsCoreInfo);

		inputManager->SetMainWindow(win);
	}


	{
		GRIND_PROFILE_SCOPE("Initialize Graphics Core");
		assetManager = new Assets::AssetManager();
		assetRendererManager = new AssetRendererManager();
		assetRendererManager->AddQueue("Opaque");
		assetRendererManager->AddQueue("Transparent");
		assetRendererManager->AddQueue("Unlit");
	}

	{
		GRIND_PROFILE_SCOPE("Load Plugin List");
		pluginManager->LoadPluginList();
	}

	{
		GRIND_PROFILE_SCOPE("Loading Default Scene");
		sceneManager = new SceneManagement::SceneManager();
		if (createInfo.shouldLoadSceneFromDefaults) {
			sceneManager->LoadDefaultScene();
		}
		else if (strcmp(createInfo.scenePath, "") == 0) {
			sceneManager->CreateEmptyScene("Untitled");
		}
		else {
			sceneManager->LoadScene(createInfo.scenePath);
		}
	}

	win->Show();

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
	GRIND_PROFILE_BEGIN_SESSION("Grindstone Running", projectPath / "log/grind-profile-run.json");
	CalculateDeltaTime();
	sceneManager->EditorUpdate();
	GRIND_PROFILE_END_SESSION();
}

void EngineCore::RunLoopIteration() {
	GRIND_PROFILE_BEGIN_SESSION("Grindstone Running", projectPath / "log/grind-profile-run.json");
	CalculateDeltaTime();
	sceneManager->Update();
	GRIND_PROFILE_END_SESSION();
}

void EngineCore::UpdateWindows() {
	windowManager->UpdateWindows();
	eventDispatcher->HandleEvents();
}

EngineCore::~EngineCore() {
	Logger::Print("Closing...");
	delete sceneManager;
	delete componentRegistrar;
	delete systemRegistrar;
	delete eventDispatcher;
	delete inputManager;
	delete pluginManager;
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
	GRIND_PROFILE_FUNC();
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
