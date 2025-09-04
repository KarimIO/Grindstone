#include <Common/Assert.hpp>
#include "ImporterSettings.hpp"

void Grindstone::Editor::ImporterSettings::SetUnknown(const std::string& key, const std::string& value) {
	GS_ASSERT(values[key].type == ImporterSettings::Type::Unknown);
	values[key] = ImporterSettings::Value{ .type = Type::Unknown, .value = value };
}

const std::string& Grindstone::Editor::ImporterSettings::Get(const std::string& key, const std::string& defaultValue) {
	auto it = values.find(std::string(key));
	if (it == values.end() || (it->second.type != Type::Unknown && it->second.type != Type::String)) {
		isDirty = true;
		values[key] = ImporterSettings::Value{ .type = Type::Uint64, .value = defaultValue };
		return values[key].value;
	}

	it->second.type = Type::String;
	return it->second.value;
}

void Grindstone::Editor::ImporterSettings::Set(const std::string& key, const std::string& value) {
	GS_ASSERT(values[key].type == ImporterSettings::Type::Unknown || values[key].type == ImporterSettings::Type::String);
	values[key] = ImporterSettings::Value{ .type=Type::String, .value=value };
	isDirty = true;
}

uint64_t Grindstone::Editor::ImporterSettings::Get(const std::string& key, uint64_t defaultValue) {
	auto it = values.find(key);
	if (it == values.end() || (it->second.type != Type::Unknown && it->second.type != Type::Uint64)) {
		isDirty = true;
		values[key] = ImporterSettings::Value{ .type=Type::Uint64, .value=std::to_string(defaultValue) };
		return defaultValue;
	}

	it->second.type = Type::Uint64;
	return static_cast<uint64_t>(std::atoll(it->second.value.c_str()));
}

void Grindstone::Editor::ImporterSettings::Set(const std::string& key, uint64_t value) {
	GS_ASSERT(values[key].type == ImporterSettings::Type::Unknown || values[key].type == ImporterSettings::Type::Uint64);
	values[key] = ImporterSettings::Value{ .type=Type::Uint64, .value=std::to_string(value) };
	isDirty = true;
}

double Grindstone::Editor::ImporterSettings::Get(const std::string& key, double defaultValue) {
	auto it = values.find(key);
	if (it == values.end() || (it->second.type != Type::Unknown && it->second.type != Type::Double)) {
		isDirty = true;
		values[key] = ImporterSettings::Value{ .type=Type::Double, .value=std::to_string(defaultValue) };
		return defaultValue;
	}

	it->second.type = Type::Double;
	return std::atof(it->second.value.c_str());
}

void Grindstone::Editor::ImporterSettings::Set(const std::string& key, double value) {
	GS_ASSERT(values[key].type == ImporterSettings::Type::Unknown || values[key].type == ImporterSettings::Type::Double);
	values[key] = ImporterSettings::Value{ .type=Type::Double, .value=std::to_string(value) };
	isDirty = true;
}

bool Grindstone::Editor::ImporterSettings::Get(const std::string& key, bool defaultValue) {
	auto it = values.find(key);
	if (it == values.end() || (it->second.type != Type::Unknown && it->second.type != Type::Bool)) {
		isDirty = true;
		values[key] = ImporterSettings::Value{ .type=Type::Bool, .value=defaultValue ? "true" : "false" };
		return defaultValue;
	}

	it->second.type = Type::Bool;
	return it->second.value == "true" ? true : false;
}

void Grindstone::Editor::ImporterSettings::Set(const std::string& key, bool value) {
	GS_ASSERT(values[key].type == ImporterSettings::Type::Unknown || values[key].type == ImporterSettings::Type::Bool);
	values[key] = ImporterSettings::Value{ .type=Type::Bool, .value=value ? "true" : "false" };
	isDirty = true;
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
