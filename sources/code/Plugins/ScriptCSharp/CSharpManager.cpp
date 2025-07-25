#include <string>

#include <nethost.h>
#include <hostfxr.h>
#include <coreclr_delegates.h>

#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/Entity.hpp>
#include <EngineCore/ECS/ComponentRegistrar.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/WorldContext/WorldContextManager.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include "Components/ScriptComponent.hpp"

#include "CSharpManager.hpp"

using namespace Grindstone;
using namespace Grindstone::Memory;
using namespace Grindstone::Scripting::CSharp;

#ifdef _DEBUG
const char* config = "Debug";
#else
const char* config = "Release";
#endif

namespace Grindstone {
	using CreateAppDomainFnPtr = void (*)(void*);
	using LoadAssemblyFnPtr = AssemblyHash(*)(void*);
	using CreateObjectFnPtr = void* (*)(AssemblyHash, void*);
	using CreateComponentFnPtr = void* (*)(AssemblyHash, void*, uint32_t);
	using CallLifetimeFnPtr = void (*)(void*);
	using DestroyObjectFnPtr = void (*)(void*);
	using VoidFnPtr = void (*)();

	struct CsharpGlobals {
		hostfxr_handle fxrHandle = nullptr;
		hostfxr_initialize_for_dotnet_command_line_fn InitForCmdLine;
		hostfxr_initialize_for_runtime_config_fn InitForConfig;
		hostfxr_get_runtime_delegate_fn GetDelegate;
		hostfxr_run_app_fn RunApp;
		hostfxr_close_fn Close;
		load_assembly_and_get_function_pointer_fn LoadAssemblyAndGetFunctionPointer = nullptr;

		CreateAppDomainFnPtr CreateAppDomain;
		VoidFnPtr UnloadAppDomain;
		LoadAssemblyFnPtr LoadAssembly;
		CreateObjectFnPtr CreateObject;
		CreateComponentFnPtr CreateComponent;
		DestroyObjectFnPtr DestroyObject;
		CallLifetimeFnPtr CallOnAttach;
		CallLifetimeFnPtr CallOnStart;
		CallLifetimeFnPtr CallOnUpdate;
		CallLifetimeFnPtr CallOnEditorUpdate;
		CallLifetimeFnPtr CallOnDestroy;
	};
}

Grindstone::CsharpGlobals csharpGlobals;

static const char* GetDotNetErrorMessage(int rc) {
	switch (rc) {
		case 0x0: return "Success";

		// ---- hostfxr return codes ----
		case 0x80008081: return "InvalidArgFailure";
		case 0x80008084: return "CoreHostLibLoadFailure";
		case 0x80008085: return "CoreHostEntryPointFailure";
		case 0x80008086: return "CoreHostCurHostFindFailure";
		case 0x80008087: return "CoreClrResolveFailure";
		case 0x80008088: return "CoreClrBindFailure";
		case 0x80008089: return "CoreClrInitFailure";
		case 0x8000808A: return "CoreClrExeFailure";
		case 0x8000808B: return "ResolverInitFailure";
		case 0x8000808C: return "ResolverResolveFailure";
		case 0x8000808D: return "LibHostCurHostFindFailure";
		case 0x80008090: return "HostApiFailed";
		case 0x80008091: return "InvalidConfigFile";
		case 0x80008092: return "AppArgNotRunnable";
		case 0x80008093: return "AppHostExeNotBoundFailure";
		case 0x80008094: return "FrameworkCompatFailure";
		case 0x80008095: return "FrameworkCompatRetry";

		// ---- Common .NET HRESULTs ----
		case 0x80131500: return "COR_E_EXCEPTION - Base class for all .NET exceptions";
		case 0x80131501: return "COR_E_TYPELOAD - Could not load the specified type";
		case 0x80131502: return "COR_E_FILELOAD - Assembly or module could not be loaded";
		case 0x80131503: return "COR_E_MISSINGMETHOD - Method not found";
		case 0x80131522: return "COR_E_ARGUMENT - One of the arguments provided is not valid";
		case 0x80131524: return "COR_E_INVALIDOPERATION - Invalid operation";
		case 0x80131534: return "COR_E_NULLREFERENCE - Null reference exception";
		case 0x80131577: return "COR_E_TARGETINVOCATION - Exception was thrown by target";

		default: return "Unknown error code";
	}
}

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
		GPRINT_ERROR_V(Grindstone::LogSource::Scripting, "Failed to get hostfxr path, returned error '{}'.", GetDotNetErrorMessage(rc));
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

	if (csharpGlobals.InitForCmdLine == nullptr ||
		csharpGlobals.InitForConfig == nullptr ||
		csharpGlobals.GetDelegate == nullptr ||
		csharpGlobals.RunApp == nullptr ||
		csharpGlobals.Close == nullptr
	) {
		GPRINT_ERROR_V(Grindstone::LogSource::Scripting, "Failed to get hostfxr functions.");
		return false;
	}

	// Temporarily create a config file to load hostfxr (for some reason I need a file to load,
	// and don't want to distribute it for security reasons.)
	{
		const char* embeddedConfigString = R"({
			"runtimeOptions": {
				"tfm": "net8.0",
				"framework": {
					"name": "Microsoft.NETCore.App",
					"version": "8.0.0"
				}
			}
		})";

		std::filesystem::path configPath = std::filesystem::temp_directory_path() / "runtimeconfig.json";
		std::ofstream out(configPath);
		out << embeddedConfigString;
		out.close();

		rc = csharpGlobals.InitForConfig(configPath.c_str(), nullptr, &csharpGlobals.fxrHandle);
		if (rc != 0 || csharpGlobals.fxrHandle == nullptr) {
			GPRINT_ERROR_V(Grindstone::LogSource::Scripting, "Failed to initialize hostfxr, returned error '{}'.", GetDotNetErrorMessage(rc));
			return false;
		}

		std::filesystem::remove(configPath);
	}

	rc = csharpGlobals.GetDelegate(
		csharpGlobals.fxrHandle,
		hdt_load_assembly_and_get_function_pointer,
		(void**)&csharpGlobals.LoadAssemblyAndGetFunctionPointer
	);

	if (rc != 0 || csharpGlobals.LoadAssemblyAndGetFunctionPointer == nullptr) {
		GPRINT_ERROR_V(Grindstone::LogSource::Scripting, "Failed to get dotnet function 'LoadAssemblyAndGetFunctionPointer', returned error '{}'.", GetDotNetErrorMessage(rc));
		return false;
	}

	return true;
}

static bool LoadGrindstoneCoreFunction(std::wstring_view dllPath, std::string_view functionName, void** fn) {
	std::wstring wfuncName = std::wstring(functionName.begin(), functionName.end());

	int rc = csharpGlobals.LoadAssemblyAndGetFunctionPointer(
		dllPath.data(),
		L"Grindstone.HostBridge, GrindstoneCSharpCore",
		wfuncName.c_str(),
		UNMANAGEDCALLERSONLY_METHOD,
		nullptr,
		fn
	);

	if (rc != 0 || fn == nullptr) {
		GPRINT_ERROR_V(Grindstone::LogSource::Scripting, "Failed to get GrindstoneCSharpCore function {}, returned error '{}'.", functionName, GetDotNetErrorMessage(rc));
		return false;
	}

	return true;
}

static bool LoadGrindstoneCoreFunctions() {
	auto coreDllPath = (EngineCore::GetInstance().GetEngineBinaryPath() / "GrindstoneCSharpCore.dll").string();
	std::wstring coreDllWide = std::wstring(coreDllPath.begin(), coreDllPath.end());
	bool hasLoadedCreateAppDomainFn = LoadGrindstoneCoreFunction(coreDllWide, "CreateAppDomain", reinterpret_cast<void**>(&csharpGlobals.CreateAppDomain));
	bool hasLoadedUnloadAppDomainFn = LoadGrindstoneCoreFunction(coreDllWide, "UnloadAppDomain", reinterpret_cast<void**>(&csharpGlobals.UnloadAppDomain));
	bool hasLoadedLoadAssemblyFn = LoadGrindstoneCoreFunction(coreDllWide, "LoadAssembly", reinterpret_cast<void**>(&csharpGlobals.LoadAssembly));
	bool hasLoadedComponentObjFn = LoadGrindstoneCoreFunction(coreDllWide, "CreateComponent", reinterpret_cast<void**>(&csharpGlobals.CreateComponent));
	bool hasLoadedCreateObjFn = LoadGrindstoneCoreFunction(coreDllWide, "CreateObject", reinterpret_cast<void**>(&csharpGlobals.CreateObject));
	bool hasLoadedDestroyObjFn = LoadGrindstoneCoreFunction(coreDllWide, "DestroyObject", reinterpret_cast<void**>(&csharpGlobals.DestroyObject));
	bool hasLoadedOnAttachFn = LoadGrindstoneCoreFunction(coreDllWide, "CallOnAttach", reinterpret_cast<void**>(&csharpGlobals.CallOnAttach));
	bool hasLoadedOnStartFn = LoadGrindstoneCoreFunction(coreDllWide, "CallOnStart", reinterpret_cast<void**>(&csharpGlobals.CallOnStart));
	bool hasLoadedOnUpdateFn = LoadGrindstoneCoreFunction(coreDllWide, "CallOnUpdate", reinterpret_cast<void**>(&csharpGlobals.CallOnUpdate));
	bool hasLoadedOnEditorUpdateFn = LoadGrindstoneCoreFunction(coreDllWide, "CallOnEditorUpdate", reinterpret_cast<void**>(&csharpGlobals.CallOnEditorUpdate));
	bool hasLoadedOnDestroyFn = LoadGrindstoneCoreFunction(coreDllWide, "CallOnDestroy", reinterpret_cast<void**>(&csharpGlobals.CallOnDestroy));

	return (
		hasLoadedCreateAppDomainFn &&
		hasLoadedUnloadAppDomainFn &&
		hasLoadedLoadAssemblyFn &&
		hasLoadedCreateObjFn &&
		hasLoadedComponentObjFn &&
		hasLoadedDestroyObjFn &&
		hasLoadedOnAttachFn &&
		hasLoadedOnStartFn &&
		hasLoadedOnUpdateFn &&
		hasLoadedOnEditorUpdateFn &&
		hasLoadedOnDestroyFn
	);
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

void CSharpManager::Initialize() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();

#ifdef _WIN32
	_putenv("DOTNET_STARTUP_HOOKS=");
	_putenv("DOTNET_EnableDiagnostics=1");
	_putenv("COMPlus_EnableDiagnostics=1");
#else
	setenv("DOTNET_STARTUP_HOOKS=");
	setenv("DOTNET_EnableDiagnostics=1");
	setenv("COMPlus_EnableDiagnostics=1");
#endif

	if (!LoadHostFxr()) {
		GPRINT_ERROR(LogSource::Scripting, "Failed to load hostfxr.");
		return;
	}

	if (!LoadGrindstoneCoreFunctions()) {
		return;
	}

	auto rootBinPath = engineCore.GetBinaryPath().string();
	csharpGlobals.CreateAppDomain((void*)rootBinPath.c_str());

	LoadAssemblyClasses();
	RegisterComponents();
}

void CSharpManager::Cleanup() {
	for (auto& smartComponent : smartComponents) {
		AllocatorCore::Free(smartComponent.second);
	}
	smartComponents.clear();

	csharpGlobals.Close(csharpGlobals.fxrHandle);
	csharpGlobals = {};
}

void CSharpManager::LoadAssembly(const char* filename, AssemblyData& outAssemblyData) {
	std::string fullFilename = std::string(filename) + ".dll";
	auto basePath = (EngineCore::GetInstance().GetBinaryPath() / fullFilename).string();
	if (!std::filesystem::exists(basePath)) {
		GPRINT_ERROR_V(LogSource::Scripting, "Attempting to load invalid assembly: {}", basePath);
		return;
	}

	if (EngineCore::GetInstance().isEditor) {
		std::filesystem::path path = basePath;
		std::filesystem::path replacePath = path;
		replacePath.replace_extension(std::string(".tmp") + path.extension().string());
		std::filesystem::copy_file(
			path,
			replacePath,
			std::filesystem::copy_options::overwrite_existing
		);

		std::filesystem::path pdbPath = basePath;
		pdbPath.replace_extension(".pdb");
		if (std::filesystem::exists(pdbPath)) {
			std::filesystem::path replacePathPath = path;
			replacePathPath.replace_extension(".tmp.pdb");
			std::filesystem::copy_file(
				pdbPath,
				replacePathPath,
				std::filesystem::copy_options::overwrite_existing
			);
		}

		std::string tmpString = replacePath.string();
		outAssemblyData.assemblyHash = csharpGlobals.LoadAssembly((void*)tmpString.c_str());
	}
	else {
		outAssemblyData.assemblyHash = csharpGlobals.LoadAssembly((void*)basePath.c_str());
	}
}

void CSharpManager::LoadAssemblyIntoMap(const char* path) {
	AssemblyData assemblyData;
	LoadAssembly(path, assemblyData);

	assemblies[path] = assemblyData;
}

void CSharpManager::SetupComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity, ScriptComponent& component) {
	std::string searchString =
		component.scriptNamespace.empty()
		? component.scriptClass
		: component.scriptNamespace + "." + component.scriptClass;
	
	auto it = assemblies.find(component.assembly);

	AssemblyHash assemblyHash;
	if (it == assemblies.end()) {
		LoadAssemblyIntoMap(component.assembly.c_str());
		assemblyHash = assemblies[component.assembly].assemblyHash;
	}
	else {
		assemblyHash = it->second.assemblyHash;
	}

	component.csharpObject = csharpGlobals.CreateComponent(assemblyHash, (void*)searchString.c_str(), (uint32_t)entity);
	csharpGlobals.CallOnAttach(component.csharpObject);
	csharpGlobals.CallOnStart(component.csharpObject);
}

void CSharpManager::DestroyComponent(Grindstone::WorldContextSet& cxtSet, entt::entity entity, ScriptComponent& component) {
	if (component.csharpObject != nullptr) {
		csharpGlobals.DestroyObject(component.csharpObject);
		component.csharpObject = nullptr;
	}
}

void CSharpManager::Update(entt::registry& registry) {
	auto view = registry.view<Grindstone::Scripting::CSharp::ScriptComponent>();
	view.each(
		[](Grindstone::Scripting::CSharp::ScriptComponent& scriptComponent) {
			if (scriptComponent.csharpObject != nullptr) {
				csharpGlobals.CallOnUpdate(scriptComponent.csharpObject);
			}
		}
	);
}

void CSharpManager::EditorUpdate(entt::registry& registry) {
	if (isReloadQueued) {
		PerformReload();
		return;
	}

	auto view = registry.view<Grindstone::Scripting::CSharp::ScriptComponent>();
	view.each(
		[](Grindstone::Scripting::CSharp::ScriptComponent& scriptComponent) {
			if (scriptComponent.csharpObject != nullptr) {
				csharpGlobals.CallOnEditorUpdate(scriptComponent.csharpObject);
			}
		}
	);
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
}

void CSharpManager::RegisterComponents() {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	auto& componentRegistrar = *engineCore.GetComponentRegistrar();
	for (auto& component : componentRegistrar) {
		const Grindstone::String& componentName = component.first.ToString();
		auto fns = component.second;
		RegisterComponent(componentName, fns);
	}
}

void CSharpManager::RegisterComponent(const Grindstone::String& componentName, ECS::ComponentFunctions& fns) {
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
	isReloadQueued = true;
}

void CSharpManager::PerformReload() {
	EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::WorldContextSet* cxtSet = engineCore.GetWorldContextManager()->GetActiveWorldContextSet();
	entt::registry& registry = engineCore.GetEntityRegistry();

	registry.view<ScriptComponent>().each([](ScriptComponent& script) {
		if (script.csharpObject != nullptr) {
			csharpGlobals.DestroyObject(script.csharpObject);
			script.csharpObject = nullptr;
		}
	});

	isReloadQueued = false;
	GPRINT_TRACE(LogSource::Scripting, "Reloading CSharp Binaries...");
	csharpGlobals.UnloadAppDomain();

	auto rootBinPath = engineCore.GetBinaryPath().string();
	csharpGlobals.CreateAppDomain((void*)rootBinPath.c_str());

	LoadAssembly("Application-CSharp", assemblies["Application-CSharp"]);
	GPRINT_TRACE(LogSource::Scripting, "Reloaded CSharp Binaries.");

	auto view = registry.view<const entt::entity, Grindstone::Scripting::CSharp::ScriptComponent>();
	view.each(
		[this, &registry, cxtSet](const entt::entity entity, Grindstone::Scripting::CSharp::ScriptComponent& scriptComponent) {
			SetupComponent(*cxtSet, entity, scriptComponent);
		}
	);
}
