#pragma once

#include <string>
#include <typeindex>

namespace Grindstone {
	using BlackboardKey = std::string;
	using BlackboardValue = char[64];
	using BlackboardTypeId = std::type_index;
	struct BlackboardEntry {
		BlackboardTypeId type;
		BlackboardValue value;

		BlackboardEntry() = delete;
		BlackboardEntry(BlackboardTypeId typeId) : type(typeId), value() {}
		BlackboardEntry(const BlackboardEntry& other) = default;
		BlackboardEntry(BlackboardEntry&& other) noexcept = default;
		BlackboardEntry& operator=(const BlackboardEntry& other) = default;
		BlackboardEntry& operator=(BlackboardEntry&& other) noexcept = default;
	};
}
