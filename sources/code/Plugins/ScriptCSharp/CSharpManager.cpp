#include "CSharpManager.hpp"

#include "Components/ScriptComponent.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

using namespace Grindstone::Scripting::CSharp;

#define FUNCTION_CALL_IMPL(CallFnInComponent, scriptMethod) \
void CSharpManager::CallFnInComponent(ScriptComponent& scriptComponent) { \
	auto offset = offsetof(ScriptClass::Methods, scriptMethod); \
	CallFunctionInComponent(scriptComponent, offset); \
}

#define FUNCTION_CALL_LIST_IMPL(CallFnInAllComponents, CallFnInComponent, scriptMethod) \
FUNCTION_CALL_IMPL(CallFnInComponent, scriptMethod) \
void CSharpManager::CallFnInAllComponents(entt::registry& registry) { \
	registry.view<ScriptComponent>().each([&](ScriptComponent& scriptComponent) { CallFnInComponent(scriptComponent); }); \
}

void CSharpManager::Initialize() {
	scriptDomain = mono_jit_init_version("grindstone_mono_domain", "v4.0.30319");

	LoadAssembly("../build/CSharpModule.dll");
}

void CSharpManager::LoadAssembly(const char* path) {
	MonoAssembly* assembly = mono_domain_assembly_open(scriptDomain, path);
	MonoImage* image = mono_assembly_get_image(assembly);
	auto& assemblyData = assemblies[path];
	assemblyData.assembly = assembly;
	assemblyData.image = image;
}

void CSharpManager::SetupComponent(ScriptComponent& component) {
	component.monoClass = SetupClass(
		component.assembly.c_str(),
		component.scriptNamespace.c_str(),
		component.scriptClass.c_str()
	);

	if (component.monoClass) {
		component.scriptObject = mono_object_new(scriptDomain, component.monoClass->monoClass);
	}
}

ScriptClass* CSharpManager::SetupClass(const char* assemblyName, const char* namespaceName, const char* className) {
	auto& assemblyIterator = assemblies.begin(); //.find(assemblyName);
	if (assemblyIterator == assemblies.end()) {
		return nullptr;
	}

	auto scriptImage = assemblyIterator->second.image;

	ScriptClass* scriptClass = new ScriptClass();
	auto& methods = scriptClass->methods;
	MonoClass* monoClass = mono_class_from_name(scriptImage, namespaceName, className);
	methods.onAttachComponent = mono_class_get_method_from_name(monoClass, "OnAttachComponent", 0);
	methods.onStart = mono_class_get_method_from_name(monoClass, "OnStart", 0);
	methods.onUpdate = mono_class_get_method_from_name(monoClass, "OnUpdate", 0);
	methods.onDelete = mono_class_get_method_from_name(monoClass, "OnDelete", 0);

	return scriptClass;
}


FUNCTION_CALL_IMPL(CallInitializeInComponent, onAttachComponent)
FUNCTION_CALL_LIST_IMPL(CallStartInAllComponents, CallStartInComponent, onStart)
FUNCTION_CALL_LIST_IMPL(CallUpdateInAllComponents, CallUpdateInComponent, onUpdate)
FUNCTION_CALL_LIST_IMPL(CallDeleteInAllComponents, CallDeleteInComponent, onDelete)

void CSharpManager::CallFunctionInComponent(ScriptComponent& scriptComponent, size_t fnOffset) {
	MonoObject* exception = nullptr;
	MonoMethod* targetMethod = (MonoMethod*)(&scriptComponent.monoClass->methods + fnOffset);
	mono_runtime_invoke(targetMethod, scriptComponent.scriptObject, nullptr, &exception);

	if (exception) {
		std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr)) << std::endl;
	}
}
