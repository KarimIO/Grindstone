#include "EngineCore.hpp"
#include "pch.hpp"

#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>
#include <EngineCore/ECS/ComponentRegistrar.hpp>
#include <EngineCore/CoreComponents/setupCoreComponents.hpp>
#include <EngineCore/CoreSystems/setupCoreSystems.hpp>
#include <EngineCore/Scenes/Manager.hpp>
#include <EngineCore/PluginSystem/DefaultPluginManager.hpp>
#include <EngineCore/Events/InputManager.hpp>
#include <EngineCore/Events/Dispatcher.hpp>
#include <EngineCore/Rendering/BaseRenderer.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/Rendering/RenderPassRegistry.hpp>
#include <EngineCore/WorldContext/WorldContextManager.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <Common/Event/WindowEvent.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Window/WindowManager.hpp>

#include "Logger.hpp"
#include "Profiling.hpp"

using namespace Grindstone;
using namespace Grindstone::Memory;

static Grindstone::EngineCore* engineCoreInstance = nullptr;

bool EngineCore::EarlyInitialize(EarlyCreateInfo& createInfo) {
	isEditor = createInfo.isEditor;
	projectPath = createInfo.projectPath;
	engineBinaryPath = createInfo.engineBinaryPath;
	binaryPath = projectPath / "bin";
	assetsPath = projectPath / "compiledAssets";
	engineAssetsPath = engineBinaryPath.parent_path() / "engineassets";

	const size_t megabytesInGig = 1024u;
	if (!AllocatorCore::Initialize(megabytesInGig * 2u)) {
		return false;
	}

	Grindstone::HashedString::CreateHashMap();
	eventDispatcher = AllocatorCore::Allocate<Events::Dispatcher>();

	firstFrameTime = std::chrono::steady_clock::now();

	profiler = &Profiler::Manager::Get();
	systemRegistrar = AllocatorCore::Allocate<ECS::SystemRegistrar>();
	componentRegistrar = AllocatorCore::Allocate<ECS::ComponentRegistrar>();

	pluginInterface = AllocatorCore::Allocate<Plugins::Interface>();
	pluginInterface->componentRegistrar = componentRegistrar;
	pluginInterface->systemRegistrar = systemRegistrar;

	Logger::Initialize(projectPath / "log" / "output.log", eventDispatcher);
	GRIND_PROFILE_BEGIN_SESSION("Grindstone Loading", projectPath / "log" / "grind-profile-load.json");
	GPRINT_INFO_V(LogSource::EngineCore, "Initializing {0}...", createInfo.applicationTitle);

	return true;
}

bool EngineCore::Initialize(LateCreateInfo& createInfo) {
	{
		GRIND_PROFILE_SCOPE("Setup Core Systems");
		SetupCoreSystems(systemRegistrar);
	}

	{
		GRIND_PROFILE_SCOPE("Setup Core Components");
		SetupCoreComponents(componentRegistrar);
	}

	// Load core (Logging, ECS and Plugin Manager)
	if (createInfo.pluginManagerOverride == nullptr) {
		pluginManager = AllocatorCore::Allocate<Plugins::DefaultPluginManager>();
	}
	else {
		pluginManager = createInfo.pluginManagerOverride;
	}

	pluginManager->PreprocessPlugins();

	pluginManager->LoadPluginsByStage("EarlyEngineSetup");

	GS_ASSERT_ENGINE(windowManager != nullptr);
	GS_ASSERT_ENGINE(graphicsCore != nullptr);

	Grindstone::Window* mainWindow = nullptr;
	{
		GRIND_PROFILE_SCOPE("Set up InputManager and Window");
		inputManager = AllocatorCore::Allocate<Input::Manager>(eventDispatcher);

		Window::CreateInfo windowCreationInfo;
		windowCreationInfo.fullscreen = Window::FullscreenMode::Windowed;
		windowCreationInfo.title = "Sandbox";
		windowCreationInfo.width = 800;
		windowCreationInfo.height = 600;
		windowCreationInfo.engineCore = this;
		windowCreationInfo.isSwapchainControlledByEngine = !isEditor;
		mainWindow = windowManager->Create(windowCreationInfo);
		eventDispatcher->AddEventListener(Grindstone::Events::EventType::WindowTryQuit, std::bind(&EngineCore::OnTryQuit, this, std::placeholders::_1));
		eventDispatcher->AddEventListener(Grindstone::Events::EventType::WindowForceQuit, std::bind(&EngineCore::OnForceQuit, this, std::placeholders::_1));
	}

	worldContextManager = AllocatorCore::Allocate<Grindstone::WorldContextManager>();

	{
		GRIND_PROFILE_SCOPE("Initialize Graphics Core");
		GraphicsAPI::Core::CreateInfo graphicsCoreInfo{ mainWindow, true };
		if (!graphicsCore->Initialize(graphicsCoreInfo)) {
			return false;
		}

		inputManager->SetMainWindow(mainWindow);
	}

	{
		GRIND_PROFILE_SCOPE("Initialize Asset Managers");
		assetManager = AllocatorCore::Allocate<Assets::AssetManager>(createInfo.assetLoader);
		assetRendererManager = AllocatorCore::Allocate<AssetRendererManager>();
	}

	{
		GRIND_PROFILE_SCOPE("Create Renderer");
		renderpassRegistry = AllocatorCore::Allocate<Grindstone::RenderPassRegistry>();
	}

	sceneManager = AllocatorCore::Allocate<SceneManagement::SceneManager>();

	GPRINT_INFO(LogSource::EngineCore, "Application Initialized.");
	GRIND_PROFILE_END_SESSION();

	pluginManager->LoadPluginsByStage("EndOfEngineSetup");

	lastFrameTime = std::chrono::steady_clock::now();

	return true;
}

void EngineCore::InitializeScene(bool shouldLoadSceneFromDefaults, const char* scenePath) {
	GRIND_PROFILE_SCOPE("Loading Default Scene");

	Grindstone::Uuid uuid;
	if (shouldLoadSceneFromDefaults) {
		sceneManager->LoadDefaultScene();
	}
	else if (strcmp(scenePath, "") == 0) {
		sceneManager->CreateEmptyScene("Untitled");
	}
	else if (!Grindstone::Uuid::MakeFromString(scenePath, uuid)) {
		sceneManager->CreateEmptyScene("Untitled");
	}
	else {
		sceneManager->LoadScene(uuid);
	}
}

void EngineCore::ShowMainWindow() {
	windowManager->GetWindowByIndex(0)->Show();
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
	systemRegistrar->EditorUpdate(GetEntityRegistry());
	GRIND_PROFILE_END_SESSION();
}

void EngineCore::RunLoopIteration() {
	GRIND_PROFILE_BEGIN_SESSION("Grindstone Running", projectPath / "log/grind-profile-run.json");
	CalculateDeltaTime();
	systemRegistrar->Update(GetEntityRegistry());
	GRIND_PROFILE_END_SESSION();
}

void EngineCore::UpdateWindows() {
	windowManager->UpdateWindows();
	eventDispatcher->HandleEvents();
}

EngineCore::~EngineCore() {
	GPRINT_INFO(LogSource::EngineCore, "Closing...");

	if (graphicsCore != nullptr) {
		graphicsCore->WaitUntilIdle();
	}

	if (windowManager != nullptr) {
		for (unsigned int i = 0; i < windowManager->GetNumWindows(); ++i) {
			windowManager->GetWindowByIndex(i)->Hide();
		}
	}

	if (sceneManager != nullptr) {
		sceneManager->CloseActiveScenes();
	}

	if (worldContextManager != nullptr) {
		worldContextManager->ClearContextSets();
	}

	if (pluginManager != nullptr) {
		pluginManager->UnloadPluginsByStage("EndOfEngineSetup");
	}

	AllocatorCore::Free(worldContextManager);
	AllocatorCore::Free(renderpassRegistry);
	AllocatorCore::Free(sceneManager);
	AllocatorCore::Free(assetRendererManager);
	AllocatorCore::Free(assetManager);
	AllocatorCore::Free(inputManager);

	if (pluginManager) {
		pluginManager->UnloadPluginsByStage("EarlyEngineSetup");
		AllocatorCore::Free(pluginManager);
	}

	AllocatorCore::Free(pluginInterface);

	AllocatorCore::Free(componentRegistrar);
	AllocatorCore::Free(systemRegistrar);
	Logger::GetLoggerState()->dispatcher = nullptr;
	AllocatorCore::Free(eventDispatcher);
	AllocatorCore::Free(Grindstone::HashedString::GetHashedStringMap());

	if (!AllocatorCore::IsEmpty()) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Uncleared memory: {0} bytes left!", static_cast<intmax_t>(AllocatorCore::GetUsed()));
	}

	GPRINT_INFO(LogSource::EngineCore, "Closed.");
	AllocatorCore::CloseAllocator();
	Logger::CloseLogger();
}

void EngineCore::RegisterGraphicsCore(GraphicsAPI::Core* newGraphicsCore) {
	graphicsCore = newGraphicsCore;
}

void EngineCore::RegisterInputManager(Input::Interface* newInputManager) {
	inputManager = newInputManager;
}

void Grindstone::EngineCore::SetRendererFactory(BaseRendererFactory* factory) {
	rendererFactory = factory;
}

Input::Interface* EngineCore::GetInputManager() const {
	return inputManager;
}

SceneManagement::SceneManager* EngineCore::GetSceneManager() const {
	return sceneManager;
}

Plugins::IPluginManager* EngineCore::GetPluginManager() const {
	return pluginManager;
}

Plugins::Interface* Grindstone::EngineCore::GetPluginInterface() const {
	return pluginInterface;
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

BaseRendererFactory* EngineCore::GetRendererFactory() const {
	return rendererFactory;
}

RenderPassRegistry* EngineCore::GetRenderPassRegistry() const {
	return renderpassRegistry;
}

WorldContextManager* EngineCore::GetWorldContextManager() const {
	return worldContextManager;
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

entt::registry& EngineCore::GetEntityRegistry() {
	return worldContextManager->GetActiveWorldContextSet()->GetEntityRegistry();
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

extern "C" {
	ENGINE_CORE_API double TimeGetTimeSinceLaunch() {
		return EngineCore::GetInstance().GetTimeSinceLaunch();
	}

	ENGINE_CORE_API double TimeGetDeltaTime() {
		return EngineCore::GetInstance().GetDeltaTime();
	}
}
