#include <iostream>
#include "TypeResolver.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_StdString : TypeDescriptor {
			TypeDescriptor_StdString()
				: TypeDescriptor{
					"std::string",
					sizeof(std::string),
					ReflectionTypeData::String
				} {}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<std::string>() {
			static TypeDescriptor_StdString typeDesc;
			return &typeDesc;
		}
	}
}
