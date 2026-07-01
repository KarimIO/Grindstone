#pragma once

#include <entt/entt.hpp>
#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/Reflection/TypeDescriptorStruct.hpp"
using namespace Grindstone;

namespace Grindstone {
	class WorldContextSet;

	namespace ECS {
		using SetupComponentFn = void(*)(Grindstone::WorldContextSet&, entt::entity);
		using DestroyComponentFn = void(*)(Grindstone::WorldContextSet&, entt::entity);
		using ClearComponentsFn = void(*)(Grindstone::WorldContextSet&);
		using GetComponentReflectionDataFn = Grindstone::Reflection::TypeDescriptor_Struct(*)();
		using TryGetComponentFn = bool(*)(Grindstone::WorldContextSet&, entt::entity, void*& outEntity);
		using HasComponentFn = bool(*)(Grindstone::WorldContextSet&, entt::entity);
		using CreateComponentFn = void*(*)(Grindstone::WorldContextSet&, entt::entity);
		using RemoveComponentFn = void(*)(Grindstone::WorldContextSet&, entt::entity);
		using CopyRegistryComponentsFn = void(*)(Grindstone::WorldContextSet& dst, Grindstone::WorldContextSet& src);
		
		class ComponentFunctions {
		public:
			SetupComponentFn SetupComponentFn = nullptr;
			DestroyComponentFn DestroyComponentFn = nullptr;
			ClearComponentsFn ClearComponentsFn = nullptr;
			CreateComponentFn CreateComponentFn = nullptr;
			RemoveComponentFn RemoveComponentFn = nullptr;
			HasComponentFn HasComponentFn = nullptr;
			TryGetComponentFn TryGetComponentFn = nullptr;
			GetComponentReflectionDataFn GetComponentReflectionDataFn = nullptr;
			CopyRegistryComponentsFn CopyRegistryComponentsFn = nullptr;
		};
	}
}
