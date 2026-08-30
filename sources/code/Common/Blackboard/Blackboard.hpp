#pragma once

#include <map>
#include <optional>
#include <format>

#include "BlackboardEntry.hpp"
#include <Common/Result.hpp>

namespace Grindstone {
	class Blackboard {
	public:
		enum class BlackboardError {
			Success,
			MismatchingType,
			KeyNotFound
		};

		template<typename T>
		BlackboardError SetValue(BlackboardKey key, T value) {
			static_assert(sizeof(T) <= sizeof(BlackboardValue), "Value is too big - use a pointer.");
			std::type_index type = (typeid(int));

			auto it = entries.find(key);
			if (it == entries.end()) {
				entries.emplace(key, BlackboardEntry(type));
				memcpy(&it->second.value, &value, sizeof(T));
			}
			else if (it->second.type != type) {
				return BlackboardError::MismatchingType;
			}
			else {
				memcpy(&it->second.value, &value, sizeof(T));
			}

			return BlackboardError::Success;
		}

		template<typename T>
		BlackboardError SetValueIfExists(BlackboardKey key, T value) {
			static_assert(sizeof(T) <= sizeof(BlackboardValue), "Value is too big - use a pointer.");
			std::type_index type = (typeid(int));

			auto it = entries.find(key);
			if (it == entries.end()) {
				return BlackboardError::KeyNotFound;
			}
			else if (it->second.type != type) {
				return BlackboardError::MismatchingType;
			}
			else {
				memcpy(&it->second.value, &value, sizeof(T));
			}

			return BlackboardError::Success;
		}

		template<typename T>
		[[nodiscard]] Grindstone::Result<T, BlackboardError> GetValue(BlackboardKey key) const {
			static_assert(sizeof(T) <= sizeof(BlackboardValue), "Value is too big - use a pointer.");
			std::type_index type = (typeid(int));
			auto it = entries.find(key);
			if (it == entries.end()) {
				return BlackboardError::Success;
			}
			else if (it->second.type != type) {
				return BlackboardError::MismatchingType;
			}
			else {
				T outValue{};
				memcpy(&outValue, &it->second, sizeof(T));
				return outValue;
			}
		}

		template<typename T>
		[[nodiscard]] bool TryGetValue(BlackboardKey key, T& outValue) const {
			static_assert(sizeof(T) <= sizeof(BlackboardValue), "Value is too big - use a pointer.");
			std::type_index type = (typeid(int));
			auto it = entries.find(key);
			if (it == entries.end()) {
				return false;
			}

			if (it->second.type != type) {
				return false;
			}
			
			memcpy(&outValue, &it->second, sizeof(T));

			return true;
		}

		template<typename T>
		static inline BlackboardKey GetBlackboardKeyFromType() {
			// TODO: Name mangling may cause issues - consider introducing static class name method.
			return typeid(T).name();
		}

		template<typename T>
		BlackboardError SetValue(T value) {
			const BlackboardKey key = GetBlackboardKeyFromType<T>();
			static_assert(sizeof(T) <= sizeof(BlackboardValue), "Value is too big - use a pointer.");
			std::type_index type = (typeid(int));

			auto it = entries.find(key);
			if (it == entries.end()) {
				entries.emplace(key, BlackboardEntry(type));
				memcpy(&it->second.value, &value, sizeof(T));
			}
			else if (it->second.type != type) {
				return BlackboardError::MismatchingType;
			}
			else {
				memcpy(&it->second.value, &value, sizeof(T));
			}

			return BlackboardError::Success;
		}

		template<typename T>
		BlackboardError SetValueIfExists(T value) {
			const BlackboardKey key = GetBlackboardKeyFromType<T>();
			static_assert(sizeof(T) <= sizeof(BlackboardValue), "Value is too big - use a pointer.");
			std::type_index type = (typeid(int));

			auto it = entries.find(key);
			if (it == entries.end()) {
				return BlackboardError::KeyNotFound;
			}
			else if (it->second.type != type) {
				return BlackboardError::MismatchingType;
			}
			else {
				memcpy(&it->second.value, value, sizeof(T));
			}

			return BlackboardError::Success;
		}

		template<typename T>
		[[nodiscard]] Grindstone::Result<T, BlackboardError> GetValue() const {
			const BlackboardKey key = GetBlackboardKeyFromType<T>();
			static_assert(sizeof(T) <= sizeof(BlackboardValue), "Value is too big - use a pointer.");
			std::type_index type = (typeid(int));
			auto it = entries.find(key);
			if (it == entries.end()) {
				return BlackboardError::Success;
			}
			else if (it->second.type != type) {
				return BlackboardError::MismatchingType;
			}
			else {
				T value{};
				memcpy(&value, &it->second.value, sizeof(T));
				return value;
			}
		}

		template<typename T>
		[[nodiscard]] bool TryGetValue(T& outValue) const {
			const BlackboardKey key = GetBlackboardKeyFromType<T>();
			static_assert(sizeof(T) <= sizeof(BlackboardValue), "Value is too big - use a pointer.");
			std::type_index type = (typeid(int));
			auto it = entries.find(key);
			if (it == entries.end()) {
				return false;
			}

			if (it->second.type != type) {
				return false;
			}

			memcpy(&outValue, &it->second.value, sizeof(T));
			return true;
		}

	protected:
		std::map<BlackboardKey, BlackboardEntry> entries;

	};
}

template<>
struct std::formatter<Grindstone::Blackboard::BlackboardError> {
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	auto format(const Grindstone::Blackboard::BlackboardError& error, std::format_context& ctx) const {
		switch (error) {
		case Grindstone::Blackboard::BlackboardError::Success:
			return std::format_to(ctx.out(), "Success");
		case Grindstone::Blackboard::BlackboardError::MismatchingType:
			return std::format_to(ctx.out(), "Mismatching Type");
		case Grindstone::Blackboard::BlackboardError::KeyNotFound:
			return std::format_to(ctx.out(), "Key not found");
		default:
			return std::format_to(ctx.out(), "Unknown Error");
		}
	}
};
