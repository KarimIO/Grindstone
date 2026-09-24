#include "DefaultResolver.hpp"
#include "Common/Rect.hpp"

namespace Grindstone::Reflection {
	struct TypeDescriptor_Box3D : TypeDescriptor {
		TypeDescriptor_Box3D() : TypeDescriptor{ "Box3D", sizeof(Grindstone::Math::Box3D), ReflectionTypeData::Box3D } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Box3D>() {
		static TypeDescriptor_Box3D typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_IntBox3D : TypeDescriptor {
		TypeDescriptor_IntBox3D() : TypeDescriptor{ "IntBox3D", sizeof(Grindstone::Math::IntBox3D), ReflectionTypeData::IntBox3D } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::IntBox3D>() {
		static TypeDescriptor_IntBox3D typeDesc;
		return &typeDesc;
	}
}
