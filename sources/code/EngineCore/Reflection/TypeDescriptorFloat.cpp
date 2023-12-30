#include "DefaultResolver.hpp"
#include "Common/Math.hpp"

namespace Grindstone::Reflection {
	struct TypeDescriptor_Float : TypeDescriptor {
		TypeDescriptor_Float() : TypeDescriptor{ "float", sizeof(float), ReflectionTypeData::Float } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<float>() {
		static TypeDescriptor_Float typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Float2 : TypeDescriptor {
		TypeDescriptor_Float2() : TypeDescriptor{ "Float2", sizeof(Math::Float2), ReflectionTypeData::Float2 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Float2>() {
		static TypeDescriptor_Float2 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Float3 : TypeDescriptor {
		TypeDescriptor_Float3() : TypeDescriptor{ "Float3", sizeof(Math::Float3), ReflectionTypeData::Float3 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Float3>() {
		static TypeDescriptor_Float3 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Float4 : TypeDescriptor {
		TypeDescriptor_Float4() : TypeDescriptor{ "Float4", sizeof(Math::Float4), ReflectionTypeData::Float4 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Float4>() {
		static TypeDescriptor_Float4 typeDesc;
		return &typeDesc;
	}
}
