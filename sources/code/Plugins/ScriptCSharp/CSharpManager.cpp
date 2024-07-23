#include <string>
#include "CSharpManager.hpp"

#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "Components/ScriptComponent.hpp"

#include <EngineCore/Utils/Utilities.hpp>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>
#include <EngineCore/Logger.hpp>

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

	std::string libPath = (engineCore->GetEngineBinaryPath().parent_path() / "deps/Mono/lib").string();
	std::string etcPath = (engineCore->GetEngineBinaryPath().parent_path() / "deps/Mono/etc").string();

	mono_set_dirs(libPath.c_str(), etcPath.c_str());

	rootDomain = mono_jit_init("GrindstoneJitRootDomain");

	mono_thread_set_main(mono_thread_current());

	CreateDomain();

	auto coreDllPath = (engineCore->GetEngineBinaryPath() / "GrindstoneCSharpCore.dll").string();
	LoadAssembly(coreDllPath.c_str(), grindstoneCoreDll);

	auto dllPath = (engineCore->GetBinaryPath() / "Application-CSharp.dll").string();
	LoadAssemblyIntoMap(dllPath.c_str());

	LoadAssemblyClasses();
	RegisterComponents();
}

void CSharpManager::Cleanup() {
	// TODO: Fix Cleanup
	return;
	mono_domain_set(mono_get_root_domain(), false);

	mono_domain_unload(scriptDomain);
	scriptDomain = nullptr;

	mono_jit_cleanup(rootDomain);
	rootDomain = nullptr;
}

void CSharpManager::CreateDomain() {
	scriptDomain = mono_domain_create_appdomain("GrindstoneAppDomain", nullptr);
	mono_domain_set(scriptDomain, true);
}

void CSharpManager::LoadAssembly(const char* path, AssemblyData& outAssemblyData) {
	std::vector<char> assemblyData = Grindstone::Utils::LoadFile(path);

	MonoImageOpenStatus status;
	MonoImage* image = mono_image_open_from_data_full(assemblyData.data(), static_cast<uint32_t>(assemblyData.size()), 1, &status, false);

	if (image == nullptr || status != MONO_IMAGE_OK) {
		const char* errorMessage = mono_image_strerror(status);
		GPRINT_ERROR(LogSource::Scripting, errorMessage);
		return;
	}

	/*
#if _DEBUG
	std::filesystem::path pdbPath = path;
	pdbPath.replace_extension(".pdb");

	if (std::filesystem::exists(pdbPath)) {
		std::vector<char> debugData = Grindstone::Utils::LoadFile(path);
		mono_debug_open_image_from_memory(image, (const mono_byte*)debugData.data(), static_cast<uint32_t>(assemblyData.size()));
	}
#endif
	*/

	MonoAssembly* assembly = mono_assembly_load_from_full(image, path, &status, 0);

	if (assembly == nullptr || status != MONO_IMAGE_OK) {
		const char* errorMessage = mono_image_strerror(status);
		GPRINT_ERROR(LogSource::Scripting, errorMessage);
		return;
	}

	mono_image_close(image);

	outAssemblyData.assembly = assembly;
	outAssemblyData.image = mono_assembly_get_image(assembly);
}

void CSharpManager::LoadAssemblyIntoMap(const char* path) {
	AssemblyData assemblyData;
	LoadAssembly(path, assemblyData);

	if (assemblyData.assembly == nullptr && assemblyData.image == nullptr) {
		return;
	}

	assemblies[path] = assemblyData;
}

struct CompactEntityData {
	entt::entity entityHandle;
	Grindstone::SceneManagement::Scene* scene;
};

void CSharpManager::SetupComponent(ECS::Entity& entity, ScriptComponent& component) {
	std::string searchString =
		component.scriptNamespace.empty()
		? component.scriptClass
		: component.scriptNamespace + "." + component.scriptClass;

	auto it = smartComponents.find(searchString);

	if (it == smartComponents.end()) {
		return;
	}

	component.monoClass = it->second;

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
	/*
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
	*/
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

	MonoImage* coreImage = grindstoneCoreDll.image;
	MonoImage* appImage = assemblies.begin()->second.image;
	const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(appImage, MONO_TABLE_TYPEDEF);
	int numTypes = mono_table_info_get_rows(typeDefinitionsTable);
	MonoClass* smartComponentClass = mono_class_from_name(coreImage, "Grindstone", "SmartComponent");

	for (int i = 0; i < numTypes; i++) {
		uint32_t cols[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

		const char* classNamespace = mono_metadata_string_heap(appImage, cols[MONO_TYPEDEF_NAMESPACE]);
		const char* className = mono_metadata_string_heap(appImage, cols[MONO_TYPEDEF_NAME]);
		std::string scriptName;
		if (strlen(classNamespace) != 0) {
			scriptName = std::string(classNamespace) + "." + className;
		}
		else {
			scriptName = className;
		}

		MonoClass* monoClass = mono_class_from_name(appImage, classNamespace, className);

		if (monoClass == smartComponentClass) {
			continue;
		}

		bool isSmartComponent = mono_class_is_subclass_of(monoClass, smartComponentClass, false);
		if (!isSmartComponent) {
			continue;
		}

		ScriptClass* scriptClass = new ScriptClass(classNamespace, className, monoClass);
		smartComponents[scriptName] = scriptClass;
		auto& methods = scriptClass->methods;
		methods.constructor = mono_class_get_method_from_name(monoClass, ".ctor", 0);
		methods.onAttachComponent = mono_class_get_method_from_name(monoClass, "OnAttachComponent", 0);
		methods.onStart = mono_class_get_method_from_name(monoClass, "OnStart", 0);
		methods.onUpdate = mono_class_get_method_from_name(monoClass, "OnUpdate", 0);
		methods.onEditorUpdate = mono_class_get_method_from_name(monoClass, "OnEditorUpdate", 0);
		methods.onDelete = mono_class_get_method_from_name(monoClass, "OnDelete", 0);

		int fieldCount = mono_class_num_fields(monoClass);
		void* iterator = nullptr;
		while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator)) {
			const char* fieldName = mono_field_get_name(field);
			uint32_t flags = mono_field_get_flags(field);
			if (flags & MONO_FIELD_ATTR_PUBLIC) {
				scriptClass->fields[fieldName] = ScriptField(fieldName, field);
				// MonoType* type = mono_field_get_type(field);

				/*
				ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
				HZ_CORE_WARN("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

				scriptClass->m_Fields[fieldName] = { fieldType, fieldName, field };
				*/
			}
		}

	}
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

// TODO: Shouldn't this be !=?
void CSharpManager::CallCreateComponent(SceneManagement::Scene* scene, entt::entity entityHandle, MonoType* monoType) {
	auto iterator = createComponentFuncs.find(monoType);
	if (iterator == createComponentFuncs.end()) {
		iterator->second(ECS::Entity(entityHandle, scene));
	}
}

// TODO: Shouldn't this be !=?
void CSharpManager::CallHasComponent(SceneManagement::Scene* scene, entt::entity entityHandle, MonoType* monoType) {
	auto iterator = hasComponentFuncs.find(monoType);
	if (iterator == hasComponentFuncs.end()) {
		iterator->second(ECS::Entity(entityHandle, scene));
	}
}

// TODO: Shouldn't this be !=?
void CSharpManager::CallRemoveComponent(SceneManagement::Scene* scene, entt::entity entityHandle, MonoType* monoType) {
	auto iterator = removeComponentFuncs.find(monoType);
	if (iterator == removeComponentFuncs.end()) {
		iterator->second(ECS::Entity(entityHandle, scene));
	}
}

void CSharpManager::QueueReload() {
	isReloadQueued = false; // TODO: make true, but it's causing an error
}

void CSharpManager::PerformReload() {
	GPRINT_TRACE(LogSource::Scripting, "Reloading CSharp Binaries...");
	mono_domain_set(mono_get_root_domain(), false);
	mono_domain_unload(scriptDomain);

	CreateDomain();

	auto coreDllPath = (engineCore->GetEngineBinaryPath() / "GrindstoneCSharpCore.dll").string();
	LoadAssembly(coreDllPath.c_str(), grindstoneCoreDll);

	auto dllPath = (engineCore->GetBinaryPath() / "Application-CSharp.dll").string();
	assemblies.clear();
	LoadAssemblyIntoMap(dllPath.c_str());
	LoadAssemblyClasses();
	RegisterComponents();
	isReloadQueued = false;
	GPRINT_TRACE(LogSource::Scripting, "Reloaded CSharp Binaries.");
}
