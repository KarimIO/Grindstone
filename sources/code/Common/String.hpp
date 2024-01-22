#pragma once

#include <string>
#include <string_view>

#define GS_TEXT(x) L ## x

namespace Grindstone {
	using String = std::wstring;
	using StringRef = std::wstring_view;
}
