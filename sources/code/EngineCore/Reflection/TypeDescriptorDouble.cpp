#include "DefaultResolver.hpp"
#include "Common/Math.hpp"

namespace Grindstone::Reflection {
	struct TypeDescriptor_Double : TypeDescriptor {
		TypeDescriptor_Double() : TypeDescriptor{ "Double", sizeof(double), ReflectionTypeData::Double } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<double>() {
		static TypeDescriptor_Double typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Double2 : TypeDescriptor {
		TypeDescriptor_Double2() : TypeDescriptor{ "Double2", sizeof(Math::Double2), ReflectionTypeData::Double2 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Double2>() {
		static TypeDescriptor_Double2 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Double3 : TypeDescriptor {
		TypeDescriptor_Double3() : TypeDescriptor{ "Double3", sizeof(Math::Double3), ReflectionTypeData::Double3 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Double3>() {
		static TypeDescriptor_Double3 typeDesc;
		return &typeDesc;
	}

	struct TypeDescriptor_Double4 : TypeDescriptor {
		TypeDescriptor_Double4() : TypeDescriptor{ "Double4", sizeof(Math::Double4), ReflectionTypeData::Double4 } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<Math::Double4>() {
		static TypeDescriptor_Double4 typeDesc;
		return &typeDesc;
	}
}
