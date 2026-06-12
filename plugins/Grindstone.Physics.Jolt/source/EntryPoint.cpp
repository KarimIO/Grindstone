#include <Grindstone.Physics.Jolt/include/pch.hpp>
#include <chrono>
#include <string>

#include <Jolt/Jolt.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/RegisterTypes.h>

#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Editor/PluginSystem/EditorPluginInterface.hpp>

#include <Grindstone.Physics.Jolt/include/Components/CharacterRigidbodyControllerComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/ColliderComponent.hpp>
#include <Grindstone.Physics.Jolt/include/Components/RigidBodyComponent.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsSystem.hpp>
#include <Grindstone.Physics.Jolt/include/PhysicsWorldContext.hpp>

#include <imgui.h>

using namespace Grindstone::Memory;
using namespace Grindstone::Physics;

class JoltPhysicsSettingsPage : public Grindstone::Editor::ImguiEditor::Settings::BasePage {
	virtual void Open() {

	}

	virtual void Render() {
		ImGui::Text("Jolt Physics is here!");
	}
};

// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char* inFMT, ...) {
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	GPRINT_TRACE(Grindstone::LogSource::Physics, buffer);
}

#ifdef JPH_ENABLE_ASSERTS

static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint32_t inLine) {
	if (inMessage == nullptr) {
		Grindstone::Logger::Print(Grindstone::LogSeverity::Error, Grindstone::LogSource::Physics, 0, inFile, inLine, "{}", inExpression);
	}
	else {
		Grindstone::Logger::Print(Grindstone::LogSeverity::Error, Grindstone::LogSource::Physics, 0, inFile, inLine, "({}) {}", inExpression, inMessage);
	}
	return true;
};

#endif // JPH_ENABLE_ASSERTS

template<typename ComponentType>
void SetupColliderComponent(Grindstone::WorldContextSet& cxt, entt::entity entity) {
	entt::registry& registry = cxt.GetEntityRegistry();
	ComponentType& colliderComponent = registry.get<ComponentType>(entity);

	CharacterRigidbodyControllerComponent* ccComponent = registry.try_get<CharacterRigidbodyControllerComponent>(entity);
	RigidBodyComponent* rigidBodyComponent = registry.try_get<RigidBodyComponent>(entity);
	TransformComponent* transformComponent = registry.try_get<TransformComponent>(entity);

	if (transformComponent != nullptr) {
		colliderComponent.Initialize(*transformComponent);
	}

	if (rigidBodyComponent != nullptr && transformComponent != nullptr) {
		SetupRigidBodyComponentWithCollider(
			cxt,
			rigidBodyComponent,
			transformComponent,
			&colliderComponent
		);
	}

	if (ccComponent != nullptr && transformComponent != nullptr) {
		SetupCharacterRigidbodyControllerComponentWithCollider(
			cxt,
			ccComponent,
			transformComponent,
			&colliderComponent
		);
	}
}

using namespace Grindstone::Memory::AllocatorCore;
static void* JoltPhysAllocate(size_t inSize) {
	constexpr size_t defaultAlignment = JPH_VECTOR_ALIGNMENT;
	return AllocatorCore::AllocateRaw(inSize, defaultAlignment, "Jolt Allocation");
}
static void JoltPhysFree(void* inBlock) { AllocatorCore::Free(inBlock); }
static void* JoltPhysAlignedAllocate(size_t inSize, size_t inAlignment) { return AllocatorCore::AllocateRaw(inSize, inAlignment, "Jolt Allocation"); }
static void JoltPhysAlignedFree(void* inBlock) { AllocatorCore::Free(inBlock); }

extern "C" {
	JOLT_PHYSICS_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
		EngineCore::SetInstance(*pluginInterface->GetEngineCore());

		JPH::Allocate = JoltPhysAllocate;
		JPH::Free = JoltPhysFree;
		JPH::AlignedAllocate = JoltPhysAlignedAllocate;
		JPH::AlignedFree = JoltPhysAlignedFree;

		// Install trace and assert callbacks
		JPH::Trace = TraceImpl;
		JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

		JPH::Factory::sInstance = new JPH::Factory();
		JPH::RegisterTypes();

		pluginInterface->RegisterComponent<BoxColliderComponent>(
			SetupColliderComponent<BoxColliderComponent>
		);

		pluginInterface->RegisterComponent<SphereColliderComponent>(
			SetupColliderComponent<SphereColliderComponent>
		);

		pluginInterface->RegisterComponent<PlaneColliderComponent>(
			SetupColliderComponent<PlaneColliderComponent>
		);

		pluginInterface->RegisterComponent<CapsuleColliderComponent>(
			SetupColliderComponent<CapsuleColliderComponent>
		);

		pluginInterface->RegisterWorldContextFactory<Grindstone::Physics::WorldContext>(physicsWorldContextName);
		pluginInterface->RegisterComponent<RigidBodyComponent>(SetupRigidBodyComponent);
		pluginInterface->RegisterComponent<CharacterRigidbodyControllerComponent>(SetupCharacterRigidbodyControllerComponent);
		// TODO: Setup CharacterRigidbody
		pluginInterface->RegisterSystem("PhysicsSystem", PhysicsJoltSystem);
	}

	JOLT_PHYSICS_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->UnregisterSystem("PhysicsSystem");
		// TODO: Remove CharacterRigidbody
		pluginInterface->UnregisterComponent<CharacterRigidbodyControllerComponent>();
		pluginInterface->UnregisterComponent<RigidBodyComponent>();
		pluginInterface->UnregisterWorldContextFactory(physicsWorldContextName);

		pluginInterface->UnregisterComponent<CapsuleColliderComponent>();
		pluginInterface->UnregisterComponent<PlaneColliderComponent>();
		pluginInterface->UnregisterComponent<SphereColliderComponent>();
		pluginInterface->UnregisterComponent<BoxColliderComponent>();

		JPH::UnregisterTypes();
		delete JPH::Factory::sInstance;
	}
}
