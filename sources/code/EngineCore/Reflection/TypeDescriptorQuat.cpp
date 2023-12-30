#include <glm/gtx/quaternion.hpp>
#include "DefaultResolver.hpp"
#include "Common/Math.hpp"

namespace Grindstone::Reflection {
	struct TypeDescriptor_Quat : TypeDescriptor {
		TypeDescriptor_Quat() : TypeDescriptor{ "quaternion", sizeof(glm::quat), ReflectionTypeData::Quaternion } {}
	};

	template <>
	TypeDescriptor* GetPrimitiveDescriptor<glm::quat>() {
		static TypeDescriptor_Quat typeDesc;
		return &typeDesc;
	}
}
