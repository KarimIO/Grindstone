#include <glm/gtx/quaternion.hpp>
#include "TypeResolver.hpp"
#include "Common/Math.hpp"

namespace Grindstone {
	namespace Reflection {
		struct TypeDescriptor_Quat : TypeDescriptor {
			TypeDescriptor_Quat() : TypeDescriptor{ "quaternion", sizeof(glm::quat), ReflectionTypeData::Quaternion } {}
		};

		template <>
		TypeDescriptor* getPrimitiveDescriptor<glm::quat>() {
			static TypeDescriptor_Quat typeDesc;
			return &typeDesc;
		}
	}
}
