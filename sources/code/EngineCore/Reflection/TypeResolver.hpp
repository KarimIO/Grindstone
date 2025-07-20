#pragma once

#include <string>
#include "TypeDescriptor.hpp"
#include "DefaultResolver.hpp"

namespace Grindstone::Reflection {
	template <typename T>
	struct TypeResolver {
		static TypeDescriptor* Get() {
			return DefaultResolver::Get<T>();
		}
	};
}
