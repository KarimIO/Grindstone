#include <iostream>
#include "TypeResolver.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_Bool : TypeDescriptor {
			TypeDescriptor_Bool() : TypeDescriptor{ "bool", sizeof(bool), ReflectionTypeData::Bool } {
			}
			virtual void dump(const void* obj, int /* unused */) const override {
				std::cout << "bool{" << *(const bool*)obj << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<bool>() {
			static TypeDescriptor_Bool typeDesc;
			return &typeDesc;
		}
	}
}
