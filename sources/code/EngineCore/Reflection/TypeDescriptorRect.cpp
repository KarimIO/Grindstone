#include "DefaultResolver.hpp"
#include "Common/Rect.hpp"

namespace Grindstone::Reflection {
	struct TypeDescriptor_Rect2D : TypeDescriptor {
		TypeDescriptor_Rect2D() : TypeDescriptor{ "Rect2D", sizeof(Grindstone::Math::Rect2D), ReflectionTypeData::Rect2D } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Rect2D>() {
		static TypeDescriptor_Rect2D typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_IntRect2D : TypeDescriptor {
		TypeDescriptor_IntRect2D() : TypeDescriptor{ "IntRect2D", sizeof(Grindstone::Math::IntRect2D), ReflectionTypeData::IntRect2D } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::IntRect2D>() {
		static TypeDescriptor_IntRect2D typeDesc;
		return &typeDesc;
	}
}
