#include "Math.hpp"

Grindstone::Math::Float3 Grindstone::Math::ImportVector(const ExportableVector inVec) {
	return { inVec.x, inVec.y, inVec.z };
}

Grindstone::Math::Quaternion Grindstone::Math::ImportQuaternion(const ExportableQuaternion inQuat) {
	return { inQuat.x, inQuat.y, inQuat.z, inQuat.w };
}

Grindstone::Math::ExportableVector Grindstone::Math::ExportVector(const Math::Float3 inVec) {
	return { inVec.x, inVec.y, inVec.z };
}

Grindstone::Math::ExportableQuaternion Grindstone::Math::ExportQuaternion(const Math::Quaternion inQuat) {
	return { inQuat.x, inQuat.y, inQuat.z, inQuat.w };
}
