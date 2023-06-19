#include <string>
#include "CSharpManager.hpp"

#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "Components/ScriptComponent.hpp"

#include <EngineCore/Utils/Utilities.hpp>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

using namespace Grindstone;
using namespace Grindstone::Scripting::CSharp;

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

	mono_set_dirs(
		"C:\\Program Files\\Mono\\lib",
		"C:\\Program Files\\Mono\\etc"
	);

	rootDomain = mono_jit_init("grindstone_mono_domain");

	CreateDomain();

	auto coreDllPath = (engineCore->GetEngineBinaryPath() / "GrindstoneCSharpCore.dll").string();
	LoadAssembly(coreDllPath.c_str(), grindstoneCoreDll);

	auto dllPath = (engineCore->GetBinaryPath() / "Application-CSharp.dll").string();
	LoadAssemblyIntoMap(dllPath.c_str());

	mono_thread_set_main(mono_thread_current());
	RegisterComponents();
}

void CSharpManager::Cleanup() {
	mono_domain_set(mono_get_root_domain(), false);

	mono_domain_unload(scriptDomain);
	scriptDomain = nullptr;

	mono_jit_cleanup(rootDomain);
	rootDomain = nullptr;
}

void CSharpManager::CreateDomain() {
	scriptDomain = mono_domain_create_appdomain("grindstone_mono_domain", nullptr);
	mono_domain_set(scriptDomain, true);
}

void CSharpManager::LoadAssembly(const char* path, AssemblyData& outAssemblyData) {
	std::vector<char> assemblyData = Grindstone::Utils::LoadFile(path);

	MonoImageOpenStatus status;
	MonoImage* image = mono_image_open_from_data_full(assemblyData.data(), static_cast<uint32_t>(assemblyData.size()), true, &status, false);

	if (image == nullptr || status != MONO_IMAGE_OK) {
		const char* errorMessage = mono_image_strerror(status);
		engineCore->Print(LogSeverity::Error, errorMessage);
		return;
	}

#if _DEBUG
	std::filesystem::path pdbPath = path;
	pdbPath.replace_extension(".pdb");

	if (std::filesystem::exists(pdbPath)) {
		std::vector<char> debugData = Grindstone::Utils::LoadFile(path);
		mono_debug_open_image_from_memory(image, (const mono_byte*)debugData.data(), static_cast<uint32_t>(assemblyData.size()));
	}
#endif

	MonoAssembly* assembly = mono_assembly_load_from_full(image, path, &status, false);

	if (assembly == nullptr || status != MONO_IMAGE_OK) {
		const char* errorMessage = mono_image_strerror(status);
		engineCore->Print(LogSeverity::Error, errorMessage);
		return;
	}

	mono_image_close(image);

	outAssemblyData.assembly = assembly;
	outAssemblyData.image = mono_assembly_get_image(assembly);
}

void CSharpManager::LoadAssemblyIntoMap(const char* path) {
	auto& assemblyData = assemblies[path];
	LoadAssembly(path, assemblyData);
}

struct CompactEntityData {
	entt::entity entityHandle;
	Grindstone::SceneManagement::Scene* scene;
};

void CSharpManager::SetupComponent(ECS::Entity& entity, ScriptComponent& component) {
	component.monoClass = SetupClass(
		component.assembly.c_str(),
		component.scriptNamespace.c_str(),
		component.scriptClass.c_str()
	);

	if (component.monoClass == nullptr || component.monoClass->monoClass == nullptr) {
		return;
	}

	component.scriptObject = mono_object_new(scriptDomain, component.monoClass->monoClass);

	SetupEntityDataInComponent(entity, component);

	CallConstructorInComponent(component);
	CallAttachComponentInComponent(component);
}

void CSharpManager::EditorUpdate(entt::registry& registry) {
	if (isReloadQueued) {
		PerformReload();
		return;
	}

	CallEditorUpdateInAllComponents(registry);
}

void CSharpManager::SetupEntityDataInComponent(ECS::Entity& entity, ScriptComponent& component) {
	MonoClassField* field = mono_class_get_field_from_name(component.monoClass->monoClass, "entity");

	if (field == nullptr) {
		return;
	}

	CompactEntityData outEnt = { entity.GetHandle(), entity.GetScene() };
	mono_field_set_value((MonoObject*)component.scriptObject, field, &outEnt);
}

ScriptClass* CSharpManager::SetupClass(const char* assemblyName, const char* namespaceName, const char* className) {
	auto& assemblyIterator = assemblies.begin(); //.find(assemblyName);
	if (assemblyIterator == assemblies.end()) {
		return nullptr;
	}

	auto scriptImage = assemblyIterator->second.image;

	if (scriptImage == nullptr) {
		return nullptr;
	}

	ScriptClass* scriptClass = new ScriptClass();
	auto& methods = scriptClass->methods;
	MonoClass* monoClass = mono_class_from_name(scriptImage, namespaceName, className);
	scriptClass->monoClass = monoClass;
	methods.constructor = mono_class_get_method_from_name(monoClass, ".ctor", 0);
	methods.onAttachComponent = mono_class_get_method_from_name(monoClass, "OnAttachComponent", 0);
	methods.onStart = mono_class_get_method_from_name(monoClass, "OnStart", 0);
	methods.onUpdate = mono_class_get_method_from_name(monoClass, "OnUpdate", 0);
	methods.onEditorUpdate = mono_class_get_method_from_name(monoClass, "OnEditorUpdate", 0);
	methods.onDelete = mono_class_get_method_from_name(monoClass, "OnDelete", 0);

	// TODO: Get Member Variables
	/*
	MonoClassField* rawField = NULL;
	void* iter = NULL;
	while ((rawField = mono_class_get_fields(monoClass, &iter)) != NULL) {
		const char* fieldName = mono_field_get_name(rawField);
		MonoType* type = mono_field_get_type(rawField);
		MonoTypeEnum monoType = (MonoTypeEnum)mono_type_get_type(type);
	}
	*/

	return scriptClass;
}

FUNCTION_CALL_IMPL(CallConstructorInComponent, constructor)
FUNCTION_CALL_IMPL(CallAttachComponentInComponent, onAttachComponent)
FUNCTION_CALL_LIST_IMPL(CallStartInAllComponents, CallStartInComponent, onStart)
FUNCTION_CALL_LIST_IMPL(CallUpdateInAllComponents, CallUpdateInComponent, onUpdate)
FUNCTION_CALL_LIST_IMPL(CallEditorUpdateInAllComponents, CallEditorUpdateInComponent, onEditorUpdate)
FUNCTION_CALL_LIST_IMPL(CallDeleteInAllComponents, CallDeleteInComponent, onDelete)


void CSharpManager::CallFunctionInComponent(ScriptComponent& scriptComponent, size_t fnOffset) {
	if (scriptComponent.monoClass == nullptr) {
		return;
	}

	MonoObject* exception = nullptr;
	char* methodsPtr = (char*)&scriptComponent.monoClass->methods;
	MonoMethod* targetMethod = *(MonoMethod**)(methodsPtr + fnOffset);
	if (targetMethod) {
		mono_runtime_invoke(targetMethod, scriptComponent.scriptObject, nullptr, &exception);

		if (exception) {
			std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr)) << std::endl;
		}
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
	MonoImage* image = grindstoneCoreDll.image;
	MonoType* managedType = mono_reflection_type_from_name((char*)csharpClass.c_str(), image);
	if (managedType == nullptr) {
		return;
	}

	createComponentFuncs[managedType] = fns.CreateComponentFn;
	tryGetComponentFuncs[managedType] = fns.TryGetComponentFn;
	hasComponentFuncs[managedType] = fns.HasComponentFn;
	removeComponentFuncs[managedType] = fns.RemoveComponentFn;
}

void CSharpManager::CallCreateComponent(SceneManagement::Scene* scene, entt::entity entityHandle, MonoType* monoType) {
	auto iterator = createComponentFuncs.find(monoType);
	if (iterator == createComponentFuncs.end()) {
		iterator->second(ECS::Entity(entityHandle, scene));
	}
}

void CSharpManager::CallHasComponent(SceneManagement::Scene* scene, entt::entity entityHandle, MonoType* monoType) {
	auto iterator = hasComponentFuncs.find(monoType);
	if (iterator == hasComponentFuncs.end()) {
		iterator->second(ECS::Entity(entityHandle, scene));
	}
}

void CSharpManager::CallRemoveComponent(SceneManagement::Scene* scene, entt::entity entityHandle, MonoType* monoType) {
	auto iterator = removeComponentFuncs.find(monoType);
	if (iterator == removeComponentFuncs.end()) {
		iterator->second(ECS::Entity(entityHandle, scene));
	}
}

void CSharpManager::QueueReload() {
	isReloadQueued = true;
}

void CSharpManager::PerformReload() {
	engineCore->Print(LogSeverity::Info, "Reloading CSharp Binaries...");
	mono_domain_set(mono_get_root_domain(), false);
	// mono_domain_unload(scriptDomain);

	CreateDomain();

	auto coreDllPath = (engineCore->GetEngineBinaryPath() / "GrindstoneCSharpCore.dll").string();
	LoadAssembly(coreDllPath.c_str(), grindstoneCoreDll);

	auto dllPath = (engineCore->GetBinaryPath() / "Application-CSharp.dll").string();
	assemblies.clear();
	LoadAssemblyIntoMap(dllPath.c_str());
	RegisterComponents();
	isReloadQueued = false;
	engineCore->Print(LogSeverity::Info, "Reloaded CSharp Binaries.");
}
