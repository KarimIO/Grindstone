#include <Grindstone.Physics.Jolt/include/pch.hpp>
#include <chrono>
#include <string>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>

#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Editor/PluginSystem/EditorPluginInterface.hpp>

#include <Grindstone.Physics.Jolt/include/Components/ColliderComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/RigidBodyComponent.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsSystem.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsWorldContext.hpp>

#include <imgui.h>

using namespace Grindstone::Memory;
using namespace Grindstone::Physics;

#ifdef _WIN32
#ifdef PHYSICS_EDITOR_DLL
#define JOLT_PHYSICS_EDITOR_EXPORT __declspec(dllexport)
#else
#define JOLT_PHYSICS_EDITOR_EXPORT __declspec(dllimport)
#endif
#else
#define JOLT_PHYSICS_EDITOR_EXPORT
#endif

class JoltPhysicsSettingsPage : public Grindstone::Editor::ImguiEditor::Settings::BasePage {
	virtual void Open() {

	}

	virtual void Render() {
		ImGui::Text("Jolt Physics is here!");
	}
};

extern "C" {
	JOLT_PHYSICS_EDITOR_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
		EngineCore::SetInstance(*pluginInterface->GetEngineCore());

		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		ImGui::SetCurrentContext(editorPluginInterface->GetImguiContext());
		if (editorPluginInterface) {
			editorPluginInterface->RegisterProjectSettingsPage(
				"Jolt Physics",
				Grindstone::Memory::AllocatorCore::AllocateUnique<JoltPhysicsSettingsPage>()
			);
		}
	}

	JOLT_PHYSICS_EDITOR_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Plugins::EditorPluginInterface* editorPluginInterface =
			static_cast<Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());

		if (editorPluginInterface) {
			editorPluginInterface->DeregisterProjectSettingsPage("Jolt Physics");
		}
	}
}
