#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Grindstone::Math {
	using Int = int;
	using Int2 = glm::tvec2<int, glm::highp>;
	using Int3 = glm::tvec3<int, glm::highp>;
	using Int4 = glm::tvec4<int, glm::highp>;

	using Uint = unsigned int;
	using Uint2 = glm::tvec2<unsigned int, glm::highp>;
	using Uint3 = glm::tvec3<unsigned int, glm::highp>;
	using Uint4 = glm::tvec4<unsigned int, glm::highp>;

	using Double = double;
	using Double2 = glm::tvec2<double, glm::highp>;
	using Double3 = glm::tvec3<double, glm::highp>;
	using Double4 = glm::tvec4<double, glm::highp>;

	using Float = float;
	using Float2 = glm::vec2;
	using Float3 = glm::vec3;
	using Float4 = glm::vec4;

	using Matrix3 = glm::mat3;
	using Matrix4 = glm::mat4;

	using Quaternion = glm::quat;
}
