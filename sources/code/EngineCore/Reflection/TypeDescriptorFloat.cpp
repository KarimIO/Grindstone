#include <iostream>
#include "TypeResolver.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_Float : TypeDescriptor {
			TypeDescriptor_Float() : TypeDescriptor{ "float", sizeof(float), ReflectionTypeData::ReflFloat } {
			}
			virtual void dump(const void* obj, int /* unused */) const override {
				std::cout << "float{" << *(const float*)obj << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<float>() {
			static TypeDescriptor_Float typeDesc;
			return &typeDesc;
		}
	}
}
