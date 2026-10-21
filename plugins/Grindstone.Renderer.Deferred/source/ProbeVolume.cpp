#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <Grindstone.Renderer.Deferred/include/ProbeVolume.hpp>

using namespace Grindstone;

REFLECT_STRUCT_BEGIN(ProbeVolume)
REFLECT_STRUCT_MEMBER(bounds)
REFLECT_STRUCT_MEMBER(probeSpacing)
REFLECT_STRUCT_MEMBER_D(probeVolumeData, decltype(T::probeVolumeData), "", "", Grindstone::Reflection::Metadata::SetInScript | Grindstone::Reflection::Metadata::ViewInAll | Grindstone::Reflection::Metadata::SetInScript, nullptr)
REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
