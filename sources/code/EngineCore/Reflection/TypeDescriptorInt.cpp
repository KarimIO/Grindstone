#include "DefaultResolver.hpp"
#include "Common/Math.hpp"

namespace Grindstone::Reflection {
	struct TypeDescriptor_Int : TypeDescriptor {
		TypeDescriptor_Int() : TypeDescriptor{ "Int", sizeof(int), ReflectionTypeData::Int } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<int>() {
		static TypeDescriptor_Int typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Int2 : TypeDescriptor {
		TypeDescriptor_Int2() : TypeDescriptor{ "Int2", sizeof(Math::Int2), ReflectionTypeData::Int2 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Int2>() {
		static TypeDescriptor_Int2 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Int3 : TypeDescriptor {
		TypeDescriptor_Int3() : TypeDescriptor{ "Int3", sizeof(Math::Int3), ReflectionTypeData::Int3 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Int3>() {
		static TypeDescriptor_Int3 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Int4 : TypeDescriptor {
		TypeDescriptor_Int4() : TypeDescriptor{ "Int4", sizeof(Math::Int4), ReflectionTypeData::Int4 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Int4>() {
		static TypeDescriptor_Int4 typeDesc;
		return &typeDesc;
	}
}
