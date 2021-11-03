#include "EngineCore/Reflection/ComponentReflection.hpp"
#include "AudioSourceComponent.hpp"
using namespace Grindstone;

REFLECT_STRUCT_BEGIN(AudioSourceComponent)
	REFLECT_STRUCT_MEMBER(audioClip)
	REFLECT_STRUCT_MEMBER(isLooping)
	REFLECT_STRUCT_MEMBER(volume)
	REFLECT_STRUCT_MEMBER(pitch)
	REFLECT_NO_SUBCAT()
REFLECT_STRUCT_END()
