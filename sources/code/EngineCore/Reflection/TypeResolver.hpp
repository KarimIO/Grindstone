#pragma once

#include <string>
#include "TypeDescriptor.hpp"

namespace Grindstone::Reflection {
	template <typename T>
	struct TypeResolver {
		static TypeDescriptor* Get() {
			return DefaultResolver::Get<T>();
		}
	};
}
