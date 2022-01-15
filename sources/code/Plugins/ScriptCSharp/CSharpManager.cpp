#include "CSharpManager.hpp"

#include "Components/ScriptComponent.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

using namespace Grindstone::Scripting::CSharp;

void CSharpManager::Initialize() {
	scriptDomain = mono_jit_init_version("grindstone_mono_domain", "v4.0.30319");

	LoadAssembly("../Main.dll");
}

void CSharpManager::LoadAssembly(const char* path) {
	MonoAssembly* assembly = mono_domain_assembly_open(scriptDomain, path);
	MonoImage* image = mono_assembly_get_image(assembly);
}

void CSharpManager::GetClass(const char* className) {
	MonoImage* image;
	MonoClass* monoClass = mono_class_from_name(image, "", className);
	auto script_method_init = mono_class_get_method_from_name(monoClass, "Initialize", 0);
	auto script_method_start = mono_class_get_method_from_name(monoClass, "Start", 0);
	auto script_method_update = mono_class_get_method_from_name(monoClass, "Update", 0);
	auto script_method_cleanup = mono_class_get_method_from_name(monoClass, "Cleanup", 0);
}

void CSharpManager::CallInitializeInComponent(ScriptComponent& scriptComponent) {
	MonoObject* exception = nullptr;
	MonoMethod* targetMethod = nullptr; // TODO: Set this
	mono_runtime_invoke(targetMethod, scriptComponent.scriptObject, nullptr, &exception);

	if (exception) {
		std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr)) << std::endl;
	}
}

void CSharpManager::CallStartInAllComponents(entt::registry& registry) {
	registry.view<ScriptComponent>().each(
		[&](
			ScriptComponent& scriptComponent
		) {
			MonoObject* exception = nullptr;
			MonoMethod* targetMethod = nullptr; // TODO: Set this
			mono_runtime_invoke(targetMethod, scriptComponent.scriptObject, nullptr, &exception);

			if (exception) {
				std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr)) << std::endl;
			}
		}
	);
}

void CSharpManager::CallUpdateInAllComponents(entt::registry& registry) {
	registry.view<ScriptComponent>().each(
		[&](
			ScriptComponent& scriptComponent
		) {
			MonoObject* exception = nullptr;
			MonoMethod* targetMethod = nullptr; // TODO: Set this
			mono_runtime_invoke(targetMethod, scriptComponent.scriptObject, nullptr, &exception);

			if (exception) {
				std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr)) << std::endl;
			}
		}
	);
}

void CSharpManager::CallCleanupInAllComponents(entt::registry& registry) {
	registry.view<ScriptComponent>().each(
		[&](
			ScriptComponent& scriptComponent
		) {
			MonoObject* exception = nullptr;
			MonoMethod* targetMethod = nullptr; // TODO: Set this
			mono_runtime_invoke(targetMethod, scriptComponent.scriptObject, nullptr, &exception);

			if (exception) {
				std::cout << mono_string_to_utf8(mono_object_to_string(exception, nullptr)) << std::endl;
			}
		}
	);
}
