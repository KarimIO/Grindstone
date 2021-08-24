#include "TypeResolver.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_Bool : TypeDescriptor {
			TypeDescriptor_Bool() : TypeDescriptor{ "bool", sizeof(bool), ReflectionTypeData::Bool } {}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<bool>() {
			static TypeDescriptor_Bool typeDesc;
			return &typeDesc;
		}
	}
}
