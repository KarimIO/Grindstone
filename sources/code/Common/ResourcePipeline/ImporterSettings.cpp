#include "ImporterSettings.hpp"

const std::string& Grindstone::Editor::ImporterSettings::Get(const std::string& key, const std::string& defaultValue) {
	auto it = values.find(std::string(key));
	if (it == values.end()) {
		values[key] = defaultValue;
		return values[key];
	}

	return it->second;
}

void Grindstone::Editor::ImporterSettings::Set(const std::string& key, const std::string& value) {
	values[key] = value;
}

const std::string& Grindstone::Editor::ImporterSettings::Get(const std::string& key, uint64_t defaultValue) {
	auto it = values.find(key);
	if (it == values.end()) {
		values[key] = std::to_string(defaultValue);
		return values[key];
	}

	return it->second;
}

void Grindstone::Editor::ImporterSettings::Set(const std::string& key, uint64_t value) {
	values[key] = std::to_string(value);
}

const std::string& Grindstone::Editor::ImporterSettings::Get(const std::string& key, double defaultValue) {
	auto it = values.find(key);
	if (it == values.end()) {
		values[key] = std::to_string(defaultValue);
		return values[key];
	}

	return it->second;
}

void Grindstone::Editor::ImporterSettings::Set(const std::string& key, double value) {
	values[key] = std::to_string(value);
}

const std::string& Grindstone::Editor::ImporterSettings::Get(const std::string& key, bool defaultValue) {
	auto it = values.find(key);
	if (it == values.end()) {
		values[key] = defaultValue ? "true" : "false";
		return values[key];
	}

	return it->second;
}

void Grindstone::Editor::ImporterSettings::Set(const std::string& key, bool value) {
	values[key] = value ? "true" : "false";
}

Grindstone::Editor::ImporterSettings::Iterator Grindstone::Editor::ImporterSettings::begin() noexcept {
	return values.begin();
}

Grindstone::Editor::ImporterSettings::ConstIterator Grindstone::Editor::ImporterSettings::begin() const noexcept {
	return values.begin();
}

Grindstone::Editor::ImporterSettings::Iterator Grindstone::Editor::ImporterSettings::end() noexcept {
	return values.end();
}

Grindstone::Editor::ImporterSettings::ConstIterator Grindstone::Editor::ImporterSettings::end() const noexcept {
	return values.end();
}
