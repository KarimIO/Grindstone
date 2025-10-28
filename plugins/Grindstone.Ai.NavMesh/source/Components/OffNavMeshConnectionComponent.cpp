#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/OffNavMeshConnectionComponent.hpp>
using namespace Grindstone::Ai;

REFLECT_STRUCT_BEGIN(OffNavMeshConnectionComponent)
REFLECT_STRUCT_MEMBER(positionA)
REFLECT_STRUCT_MEMBER(positionB)
REFLECT_STRUCT_MEMBER(radius)
REFLECT_STRUCT_MEMBER_D(direction, uint8_t, "", "", Grindstone::Reflection::Metadata::SaveSetAndView, nullptr)
REFLECT_STRUCT_MEMBER(areaId)
REFLECT_STRUCT_MEMBER(flags)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
