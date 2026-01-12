#include "DefaultResolver.hpp"
#include <Common/PhysicsLayer.hpp>

namespace Grindstone::Reflection {
	struct TypeDescriptor_PhysicsLayer : TypeDescriptor {
		TypeDescriptor_PhysicsLayer() : TypeDescriptor{ "PhysicsLayer", sizeof(Grindstone::Physics::Layer), ReflectionTypeData::PhysicsLayer } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Grindstone::Physics::Layer>() {
		static TypeDescriptor_PhysicsLayer typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_PhysicsLayerMask : TypeDescriptor {
		TypeDescriptor_PhysicsLayerMask() : TypeDescriptor{ "PhysicsLayerMask", sizeof(Grindstone::Physics::LayerMask), ReflectionTypeData::PhysicsLayerMask } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Grindstone::Physics::LayerMask>() {
		static TypeDescriptor_PhysicsLayerMask typeDesc;
		return &typeDesc;
	}
}
