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

	colliderComponent.Initialize();

	RigidBodyComponent* rigidBodyComponent = registry.try_get<RigidBodyComponent>(entity);
	TransformComponent* transformComponent = registry.try_get<TransformComponent>(entity);
	if (rigidBodyComponent != nullptr && transformComponent != nullptr) {
		SetupRigidBodyComponentWithCollider(
			cxt,
			rigidBodyComponent,
			transformComponent,
			&colliderComponent
		);
	}
}

extern "C" {
	JOLT_PHYSICS_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
		EngineCore::SetInstance(*pluginInterface->GetEngineCore());

		// Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
		// This needs to be done before any other Jolt function is called.
		JPH::RegisterDefaultAllocator();

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
		pluginInterface->RegisterSystem("PhysicsSystem", PhysicsJoltSystem);
	}

	JOLT_PHYSICS_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->UnregisterSystem("PhysicsSystem");
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
