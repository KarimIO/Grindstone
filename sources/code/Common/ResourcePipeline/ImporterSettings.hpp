#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace Grindstone::Editor {
	struct ImporterSettings {
		enum class Type : uint8_t {
			Unknown = 0,
			String,
			Uint64,
			Double,
			Bool
		};
		
		struct Value {
			Grindstone::Editor::ImporterSettings::Type type;
			std::string value;
		};

		using ValueMap = std::unordered_map<std::string, Value>;
		ValueMap values;

		void SetUnknown(const std::string& key, const std::string& value);

		const std::string& Get(const std::string& key, const std::string& defaultValue);
		void Set(const std::string& key, const std::string& value);

		uint64_t Get(const std::string& key, uint64_t defaultValue);
		void Set(const std::string& key, uint64_t value);

		double Get(const std::string& key, double defaultValue);
		void Set(const std::string& key, double value);

		bool Get(const std::string& key, bool defaultValue);
		void Set(const std::string& key, bool value);

		using Iterator = ValueMap::iterator;
		using ConstIterator = ValueMap::const_iterator;

		Iterator begin() noexcept;
		ConstIterator begin() const noexcept;

		Iterator end() noexcept;
		ConstIterator end() const noexcept;

		bool isDirty;
	};
}
