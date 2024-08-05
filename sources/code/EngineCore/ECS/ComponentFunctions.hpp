#pragma once

#include <entt/entt.hpp>
#include "EngineCore/ECS/Entity.hpp"
#include "EngineCore/Reflection/TypeDescriptorStruct.hpp"
using namespace Grindstone;

namespace Grindstone {
	namespace ECS {
		using SetupComponentFn = void(*)(entt::registry&, entt::entity);
		using DestroyComponentFn = void(*)(entt::registry&, entt::entity);
		using GetComponentReflectionDataFn = Grindstone::Reflection::TypeDescriptor_Struct(*)();
		using TryGetComponentFn = bool(*)(entt::registry&, entt::entity, void*& outEntity);
		using HasComponentFn = bool(*)(entt::registry&, entt::entity);
		using CreateComponentFn = void*(*)(entt::registry&, entt::entity);
		using RemoveComponentFn = void(*)(entt::registry&, entt::entity);
		using CopyRegistryComponentsFn = void(*)(entt::registry& dst, entt::registry& src);
		
		class ComponentFunctions {
		public:
			SetupComponentFn SetupComponentFn = nullptr;
			DestroyComponentFn DestroyComponentFn = nullptr;
			CreateComponentFn CreateComponentFn = nullptr;
			RemoveComponentFn RemoveComponentFn = nullptr;
			HasComponentFn HasComponentFn = nullptr;
			TryGetComponentFn TryGetComponentFn = nullptr;
			GetComponentReflectionDataFn GetComponentReflectionDataFn = nullptr;
			CopyRegistryComponentsFn CopyRegistryComponentsFn = nullptr;
		};
	}
}
