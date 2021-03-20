#include <iostream>
#include "TypeResolver.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_Vec3 : TypeDescriptor {
			TypeDescriptor_Vec3() : TypeDescriptor{ "vec3", sizeof(Math::Vec3), ReflectionTypeData::ReflVec3 } {
			}
			virtual void dump(const void* obj, int /* unused */) const override {
				const Math::Vec3& vec3 = *(const Math::Vec3*)obj;
				std::cout << "Vec3{" << vec3.x << ", " << vec3.y << ", " << vec3.z << "}";
			}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<Math::Vec3>() {
			static TypeDescriptor_Vec3 typeDesc;
			return &typeDesc;
		}
	}
}
