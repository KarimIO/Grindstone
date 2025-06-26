#include <string>

#include <nethost.h>
#include <hostfxr.h>

#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/Entity.hpp>
#include <EngineCore/ECS/ComponentRegistrar.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "Components/ScriptComponent.hpp"

#include "CSharpManager.hpp"

using namespace Grindstone;
using namespace Grindstone::Memory;
using namespace Grindstone::Scripting::CSharp;

namespace Grindstone {
	struct CsharpGlobals {
		hostfxr_handle fxrHandle = nullptr;
		hostfxr_initialize_for_dotnet_command_line_fn InitForCmdLine;
		hostfxr_initialize_for_runtime_config_fn InitForConfig;
		hostfxr_get_runtime_delegate_fn GetDelegate;
		hostfxr_run_app_fn RunApp;
		hostfxr_close_fn Close;
	};
}

Grindstone::CsharpGlobals csharpGlobals;

#ifdef WIN32
static void* LoadModuleFile(const char_t* path) {
	HMODULE h = ::LoadLibraryW(path);
	assert(h != nullptr);
	return (void*)h;
}

static void* GetModuleExport(void* h, const char* name) {
	void* f = ::GetProcAddress((HMODULE)h, name);
	assert(f != nullptr);
	return f;
}
#else
static void* LoadModuleFile(const char_t* path) {
	void* h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
	assert(h != nullptr);
	return h;
}

static void* GetModuleExport(void* h, const char* name) {
	void* f = dlsym(h, name);
	assert(f != nullptr);
	return f;
}
#endif

static bool LoadHostFxr() {
	get_hostfxr_parameters params{ sizeof(get_hostfxr_parameters), nullptr, nullptr };
	// Pre-allocate a large buffer for the path to hostfxr
	char_t buffer[MAX_PATH];
	size_t bufferSize = sizeof(buffer) / sizeof(char_t);
	int rc = get_hostfxr_path(buffer, &bufferSize, &params);
	if (rc != 0) {
		return false;
	}

	// Load hostfxr and get desired exports
	// NOTE: The .NET Runtime does not support unloading any of its native libraries. Running
	// dlclose/FreeLibrary on any .NET libraries produces undefined behavior.
	void* lib = LoadModuleFile(buffer);
	csharpGlobals.InitForCmdLine = (hostfxr_initialize_for_dotnet_command_line_fn)GetModuleExport(lib, "hostfxr_initialize_for_dotnet_command_line");
	csharpGlobals.InitForConfig = (hostfxr_initialize_for_runtime_config_fn)GetModuleExport(lib, "hostfxr_initialize_for_runtime_config");
	csharpGlobals.GetDelegate = (hostfxr_get_runtime_delegate_fn)GetModuleExport(lib, "hostfxr_get_runtime_delegate");
	csharpGlobals.RunApp = (hostfxr_run_app_fn)GetModuleExport(lib, "hostfxr_run_app");
	csharpGlobals.Close = (hostfxr_close_fn)GetModuleExport(lib, "hostfxr_close");

	return (csharpGlobals.InitForCmdLine && csharpGlobals.GetDelegate && csharpGlobals.Close);
}

#define FUNCTION_CALL_IMPL(CallFnInComponent, scriptMethod) \
void CSharpManager::CallFnInComponent(ScriptComponent& scriptComponent) { \
	auto offset = offsetof(ScriptClass::Methods, scriptMethod); \
	CallFunctionInComponent(scriptComponent, offset); \
}

#define FUNCTION_CALL_LIST_IMPL(CallFnInAllComponents, CallFnInComponent, scriptMethod) \
FUNCTION_CALL_IMPL(CallFnInComponent, scriptMethod) \
void CSharpManager::CallFnInAllComponents(entt::registry& registry) { \
	auto fnCall = [&](ScriptComponent& scriptComponent) { CallFnInComponent(scriptComponent); }; \
	registry.view<ScriptComponent>().each(fnCall); \
}

CSharpManager& CSharpManager::GetInstance() {
	static CSharpManager instance;
	return instance;
}

void CSharpManager::Initialize(EngineCore* engineCore) {
	this->engineCore = engineCore;

	if (!LoadHostFxr()) {
		GPRINT_ERROR(LogSource::Scripting, "Failed to load hostfxr.");
		return;
	}

	auto coreDllPath = (engineCore->GetEngineBinaryPath() / "GrindstoneCSharpCore.dll").string();
	LoadAssembly(coreDllPath.c_str(), grindstoneCoreDll);

	auto dllPath = (engineCore->GetBinaryPath() / "Application-CSharp.dll").string();
	LoadAssemblyIntoMap(dllPath.c_str());

	LoadAssemblyClasses();
	RegisterComponents();
}

void CSharpManager::Cleanup() {
	for (auto& smartComponent : smartComponents) {
		AllocatorCore::Free(smartComponent.second);
	}
	smartComponents.clear();

	csharpGlobals.Close(csharpGlobals.fxrHandle);
}

void CSharpManager::LoadAssembly(const char* path, AssemblyData& outAssemblyData) {
	if (!std::filesystem::exists(path)) {
		GPRINT_ERROR_V(LogSource::Scripting, "Attempting to load invalid assembly: {}", path);
		return;
	}

}

void CSharpManager::LoadAssemblyIntoMap(const char* path) {
	AssemblyData assemblyData;
	LoadAssembly(path, assemblyData);

	assemblies[path] = assemblyData;
}

void CSharpManager::SetupComponent(entt::registry& registry, entt::entity entity, ScriptComponent& component) {
	std::string searchString =
		component.scriptNamespace.empty()
		? component.scriptClass
		: component.scriptNamespace + "." + component.scriptClass;

	auto it = smartComponents.find(searchString);

	if (it == smartComponents.end()) {
		return;
	}

}

void CSharpManager::DestroyComponent(entt::registry& registry, entt::entity entity, ScriptComponent& component) {
	
}

void CSharpManager::EditorUpdate(entt::registry& registry) {
	if (isReloadQueued) {
		PerformReload();
		return;
	}
}

void CSharpManager::SetupEntityDataInComponent(entt::entity entity, ScriptComponent& component) {
}

ScriptClass* CSharpManager::SetupClass(const char* assemblyName, const char* namespaceName, const char* className) {
	return nullptr;
}

void CSharpManager::LoadAssemblyClasses() {
	for(auto& comp : smartComponents) {
		delete comp.second;
	}

	if (assemblies.empty()) {
		return;
	}

	smartComponents.clear();

}

void CSharpManager::CallFunctionInComponent(ScriptComponent& scriptComponent, size_t fnOffset) {
	if (scriptComponent.monoClass == nullptr) {
		return;
	}

}

void CSharpManager::RegisterComponents() {
	auto& componentRegistrar = *engineCore->GetComponentRegistrar();
	for (auto& component : componentRegistrar) {
		auto& componentName = (std::string&)component.first;
		auto fns = component.second;
		RegisterComponent(componentName, fns);
	}
}

void CSharpManager::RegisterComponent(std::string& componentName, ECS::ComponentFunctions& fns) {
	std::string csharpClass = "Grindstone." + componentName + "Component";
}

// TODO: Shouldn't this be !=?
void CSharpManager::CallCreateComponent(SceneManagement::Scene* scene, entt::entity entity) {
}

// TODO: Shouldn't this be !=?
void CSharpManager::CallHasComponent(SceneManagement::Scene* scene, entt::entity entity) {
}

// TODO: Shouldn't this be !=?
void CSharpManager::CallRemoveComponent(SceneManagement::Scene* scene, entt::entity entity) {
}

void CSharpManager::QueueReload() {
	isReloadQueued = false; // TODO: make true, but it's causing an error
}

void CSharpManager::PerformReload() {
	GPRINT_TRACE(LogSource::Scripting, "Reloading CSharp Binaries...");
	GPRINT_TRACE(LogSource::Scripting, "Reloaded CSharp Binaries.");
}
