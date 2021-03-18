#include <iostream>
#include "TypeResolver.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_Int : TypeDescriptor {
			TypeDescriptor_Int() : TypeDescriptor{ "int", sizeof(int), ReflectionTypeData::ReflInt } {
			}
			virtual void dump(const void* obj, int /* unused */) const override {
				std::cout << "int{" << *(const int*)obj << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<int>() {
			static TypeDescriptor_Int typeDesc;
			return &typeDesc;
		}
	}
}
