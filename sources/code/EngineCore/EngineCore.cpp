#include "pch.hpp"

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
#include "Common/Window/WindowManager.hpp"
#include "Events/Dispatcher.hpp"
#include "Common/Event/WindowEvent.hpp"

#include "Rendering/DeferredRenderer.hpp"

#include "EngineCore/AssetRenderer/AssetRendererManager.hpp"
#include "Assets/AssetManager.hpp"

using namespace Grindstone;

bool EngineCore::Initialize(CreateInfo& createInfo) {
	isEditor = createInfo.isEditor;
	projectPath = createInfo.projectPath;
	engineBinaryPath = createInfo.engineBinaryPath;
	binaryPath = projectPath / "bin/";
	assetsPath = projectPath / "compiledAssets/";
	engineAssetsPath = engineBinaryPath.parent_path() / "engineassets";

	const size_t megabytesInGig = 1024u;
	if (!memoryAllocator.Initialize(megabytesInGig * 1u)) {
		return false;
	}

	eventDispatcher = memoryAllocator.Allocate<Events::Dispatcher>();

	firstFrameTime = std::chrono::steady_clock::now();

	profiler = &Profiler::Manager::Get();

	Logger::Initialize(projectPath / "log/output.log", eventDispatcher);
	GRIND_PROFILE_BEGIN_SESSION("Grindstone Loading", projectPath / "log/grind-profile-load.json");
	GPRINT_INFO_V(LogSource::EngineCore, "Initializing {0}...", createInfo.applicationTitle);

	{
		GRIND_PROFILE_SCOPE("Setup Core Systems");
		systemRegistrar = memoryAllocator.Allocate<ECS::SystemRegistrar>();
		SetupCoreSystems(systemRegistrar);
	}

	{
		GRIND_PROFILE_SCOPE("Setup Core Components");
		componentRegistrar = memoryAllocator.Allocate<ECS::ComponentRegistrar>();
		SetupCoreComponents(componentRegistrar);
	}

	// Load core (Logging, ECS and Plugin Manager)
	pluginManager = memoryAllocator.Allocate<Plugins::Manager>(this);
	pluginManager->GetInterface().SetEditorInterface(createInfo.editorPluginInterface);
	pluginManager->Load("PluginGraphicsVulkan");

	Grindstone::Window* mainWindow = nullptr;
	{
		GRIND_PROFILE_SCOPE("Set up InputManager and Window");
		inputManager = memoryAllocator.Allocate<Input::Manager>(eventDispatcher);

		Window::CreateInfo windowCreationInfo;
		windowCreationInfo.fullscreen = Window::FullscreenMode::Windowed;
		windowCreationInfo.title = "Sandbox";
		windowCreationInfo.width = 800;
		windowCreationInfo.height = 600;
		windowCreationInfo.engineCore = this;
		windowCreationInfo.isSwapchainControlledByEngine = !createInfo.isEditor;
		mainWindow = windowManager->Create(windowCreationInfo);
		eventDispatcher->AddEventListener(Grindstone::Events::EventType::WindowTryQuit, std::bind(&EngineCore::OnTryQuit, this, std::placeholders::_1));
		eventDispatcher->AddEventListener(Grindstone::Events::EventType::WindowForceQuit, std::bind(&EngineCore::OnForceQuit, this, std::placeholders::_1));
	}

	{
		GRIND_PROFILE_SCOPE("Initialize Graphics Core");
		GraphicsAPI::Core::CreateInfo graphicsCoreInfo{ mainWindow, true };
		graphicsCore->Initialize(graphicsCoreInfo);

		inputManager->SetMainWindow(mainWindow);
	}

	{
		GRIND_PROFILE_SCOPE("Initialize Asset Managers");
		assetManager = memoryAllocator.Allocate<Assets::AssetManager>(createInfo.assetLoader);
		assetRendererManager = memoryAllocator.Allocate<AssetRendererManager>();
		assetRendererManager->AddQueue("Opaque", DrawSortMode::DistanceFrontToBack);
		assetRendererManager->AddQueue("Transparent", DrawSortMode::DistanceBackToFront);
		assetRendererManager->AddQueue("Unlit", DrawSortMode::DistanceFrontToBack);
		assetRendererManager->AddQueue("Skybox", DrawSortMode::DistanceFrontToBack);
	}

	{
		GRIND_PROFILE_SCOPE("Load Plugin List");
		pluginManager->LoadPluginList();
	}

	sceneManager = memoryAllocator.Allocate<SceneManagement::SceneManager>();

	GPRINT_INFO_V(LogSource::EngineCore, "{0} Initialized.", createInfo.applicationTitle);
	GRIND_PROFILE_END_SESSION();

	lastFrameTime = std::chrono::steady_clock::now();

	return true;
}

void EngineCore::InitializeScene(bool shouldLoadSceneFromDefaults, const char* scenePath) {
	GRIND_PROFILE_SCOPE("Loading Default Scene");
	if (shouldLoadSceneFromDefaults) {
		sceneManager->LoadDefaultScene();
	}
	else if (strcmp(scenePath, "") == 0) {
		sceneManager->CreateEmptyScene("Untitled");
	}
	else {
		sceneManager->LoadScene(scenePath);
	}
}

void EngineCore::ShowMainWindow() {
	windowManager->GetWindowByIndex(0)->Show();
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
	assetManager->ReloadQueuedAssets();
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
	GPRINT_INFO(LogSource::EngineCore, "Closing...");

	memoryAllocator.Free(sceneManager);
	memoryAllocator.Free(assetRendererManager);
	memoryAllocator.Free(assetManager);
	memoryAllocator.Free(inputManager);
	memoryAllocator.Free(pluginManager);
	memoryAllocator.Free(componentRegistrar);
	memoryAllocator.Free(systemRegistrar);
	memoryAllocator.Free(eventDispatcher);

	if (!memoryAllocator.IsEmpty()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Uncleared memory: {0} bytes left!", memoryAllocator.GetUsed());
	}

	GPRINT_INFO(LogSource::EngineCore, "Closed.");
	Logger::CloseLogger();
}

void EngineCore::RegisterGraphicsCore(GraphicsAPI::Core* newGraphicsCore) {
	graphicsCore = newGraphicsCore;
}

void EngineCore::RegisterInputManager(Input::Interface* newInputManager) {
	inputManager = newInputManager;
}

Input::Interface* EngineCore::GetInputManager() const {
	return inputManager;
}

SceneManagement::SceneManager* EngineCore::GetSceneManager() const {
	return sceneManager;
}

Plugins::Manager* EngineCore::GetPluginManager() const {
	return pluginManager;
}

ECS::ComponentRegistrar* EngineCore::GetComponentRegistrar() const {
	return componentRegistrar;
}

GraphicsAPI::Core* EngineCore::GetGraphicsCore() const {
	return graphicsCore;
}

ECS::SystemRegistrar* EngineCore::GetSystemRegistrar() const {
	return systemRegistrar;
}

Events::Dispatcher* EngineCore::GetEventDispatcher() const {
	return eventDispatcher;
}

BaseRenderer* EngineCore::CreateRenderer(GraphicsAPI::RenderPass* targetRenderPass) {
	return new DeferredRenderer(targetRenderPass);
}

std::filesystem::path EngineCore::GetProjectPath() const {
	return projectPath;
}

std::filesystem::path EngineCore::GetBinaryPath() const {
	return binaryPath;
}

std::filesystem::path EngineCore::GetEngineBinaryPath() const {
	return engineBinaryPath;
}

std::filesystem::path EngineCore::GetAssetsPath() const {
	return assetsPath;
}

std::filesystem::path EngineCore::GetEngineAssetsPath() const {
	return engineAssetsPath;
}

std::filesystem::path EngineCore::GetAssetPath(std::string subPath) const {
	return assetsPath / subPath;
}

Profiler::Manager* EngineCore::GetProfiler() const {
	return profiler;
}

bool EngineCore::OnTryQuit(Grindstone::Events::BaseEvent* ev) {
	const auto castedEv = dynamic_cast<Grindstone::Events::WindowTryQuitEvent*>(ev);
	shouldClose = true;

	return false;
}

bool EngineCore::OnForceQuit(Grindstone::Events::BaseEvent* ev) {
	const auto castedEv = dynamic_cast<Grindstone::Events::WindowTryQuitEvent*>(ev);
	shouldClose = true;

	return false;
}

void EngineCore::ReloadCsharpBinaries() {
	if (callbackReloadCsharp) {
		callbackReloadCsharp();
	}
}

void EngineCore::CalculateDeltaTime() {
	GRIND_PROFILE_FUNC();
	const auto now = std::chrono::steady_clock::now();

	const auto elapsedTimeSinceLastFrame = now - lastFrameTime;
	const auto elapsedNsSinceLastFrame = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsedTimeSinceLastFrame).count();
	deltaTime = static_cast<double>(elapsedNsSinceLastFrame) * 0.000000001;

	const auto elapsedTimeSinceFirstTime = now - lastFrameTime;
	const auto elapsedNsSinceFirstTime = std::chrono::duration_cast<std::chrono::nanoseconds>(elapsedTimeSinceFirstTime).count();
	currentTime = static_cast<double>(elapsedNsSinceFirstTime) * 0.000000001;

	lastFrameTime = now;
}

double EngineCore::GetTimeSinceLaunch() const {
	return currentTime;
}

double EngineCore::GetDeltaTime() const {
	return deltaTime;
}

Memory::AllocatorCore& Grindstone::EngineCore::GetAllocator() {
	return memoryAllocator;
}

extern "C" {
	ENGINE_CORE_API double TimeGetTimeSinceLaunch() {
		return EngineCore::GetInstance().GetTimeSinceLaunch();
	}

	ENGINE_CORE_API double TimeGetDeltaTime() {
		return EngineCore::GetInstance().GetDeltaTime();
	}
}
