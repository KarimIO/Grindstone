#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/NavAgentComponent.hpp>
using namespace Grindstone::Ai;

REFLECT_STRUCT_BEGIN(NavAgentComponent)
	REFLECT_STRUCT_MEMBER(typeId)
	REFLECT_STRUCT_MEMBER(baseOffset)
	REFLECT_STRUCT_MEMBER(movementSpeed)
	REFLECT_STRUCT_MEMBER(angularSpeed)
	REFLECT_STRUCT_MEMBER(acceleration)
	REFLECT_STRUCT_MEMBER(stoppingDistance)
	REFLECT_STRUCT_MEMBER(autoBraking)
	REFLECT_STRUCT_MEMBER(obstacleAvoidanceRadius)
	REFLECT_STRUCT_MEMBER(obstacleAvoidanceHeight)
	REFLECT_STRUCT_MEMBER(obstacleAvoidanceQuality)
	REFLECT_STRUCT_MEMBER(obstacleAvoidancePriority)
	REFLECT_STRUCT_MEMBER(autoTraverseOffMesh)
	REFLECT_STRUCT_MEMBER(autoRepath)
	REFLECT_STRUCT_MEMBER(areaMask)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
