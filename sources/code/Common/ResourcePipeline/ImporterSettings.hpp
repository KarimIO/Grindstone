#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace Grindstone::Editor {
	struct ImporterSettings {
		using ValueMap = std::unordered_map<std::string, std::string>;
		ValueMap values;

		const std::string& Get(const std::string& key, const std::string& defaultValue);
		void Set(const std::string& key, const std::string& value);

		const std::string& Get(const std::string& key, uint64_t defaultValue);
		void Set(const std::string& key, uint64_t value);

		const std::string& Get(const std::string& key, double defaultValue);
		void Set(const std::string& key, double value);

		const std::string& Get(const std::string& key, bool defaultValue);
		void Set(const std::string& key, bool value);

		using Iterator = ValueMap::iterator;
		using ConstIterator = ValueMap::const_iterator;

		Iterator begin() noexcept;
		ConstIterator begin() const noexcept;

		Iterator end() noexcept;
		ConstIterator end() const noexcept;
	};
}
