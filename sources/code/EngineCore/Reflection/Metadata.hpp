#pragma once

#include <Common/EnumTraits.hpp>

namespace Grindstone::Reflection {
	enum class Metadata : short {
		NoMetadata = 0,
		ViewInEditor = 1 << 0,
		SetInEditor = 1 << 1,
		ViewInScript = 1 << 2,
		SetInScript = 1 << 3,
		ViewInAll = ViewInEditor | ViewInScript,
		SetInAll = SetInEditor | SetInScript,
		SaveState = 1 << 4,
		SaveSetAndView = ViewInAll | SetInAll | SaveState
	};

	GS_ENUM_FLAGS_FUNCS(Grindstone::Reflection::Metadata);
}
