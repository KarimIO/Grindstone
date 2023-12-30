#include <entt/fwd.hpp>

#include "DefaultResolver.hpp"

namespace Grindstone::Reflection {
	struct TypeDescriptor_Entity : TypeDescriptor {
		TypeDescriptor_Entity() : TypeDescriptor{ "entt::entity", sizeof(entt::entity), ReflectionTypeData::Entity } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<entt::entity>() {
		static TypeDescriptor_Entity typeDesc;
		return &typeDesc;
	}
}
