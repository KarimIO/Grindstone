#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/NavMeshComponent.hpp>
using namespace Grindstone::Ai;

REFLECT_STRUCT_BEGIN(NavMeshComponent)
	REFLECT_STRUCT_MEMBER(cellSize)
	REFLECT_STRUCT_MEMBER(cellHeight)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
